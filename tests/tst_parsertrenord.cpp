// Test del parser degli avvisi Trenord con risposte reali del servizio
// https://www.trenord.it/rest/render/{shoulder-lines,line-details} (scaricate il 07/10/2026)
// e con alcuni casi sintetici
#include "parser_trenord.h"

#include <QFile>
#include <QtTest>

namespace {

QString leggiFixture(const QString &nome)
{
    QFile file(QStringLiteral(QVT_TEST_DATA "/") + nome);
    if (!file.open(QIODevice::ReadOnly))
        return QString();
    return QString::fromUtf8(file.readAll());
}

// frammento di lista linee nel formato del servizio, con una sola linea
QString lineaSintetica(const QString &codice, const QString &classeStato, const QString &tooltip)
{
    return QStringLiteral(
               "<li><a href=\"/x?code=%1\" data-code=\"%1\" data-direttrice=\"D001\" data-name=\"Linea %1\">"
               "<div><div class=\"crop\"><p>Linea %1</p></div><span>"
               "<div class=\" status-line no-margin %2 wrapper\"><span class=\"dot\"></span>"
               "<div class=\"tooltip\">%3</div></div></span></div></a></li>")
        .arg(codice, classeStato, tooltip);
}

QString comeJson(const QString &html)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("message"), html);
    return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

} // namespace

class TstParserTrenord : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void listaLineeReale()
    {
        const QString risposta = leggiFixture("trenord_linee.json");
        QVERIFY(!risposta.isEmpty());

        ParserTrenord parser(nullptr);
        QVERIFY(parser.analizzaListaDirettrici(risposta));

        const QList<TrenordVT::Linea> linee = parser.linee();
        QCOMPARE(linee.size(), 65);

        const TrenordVT::Linea prima = linee.first();
        QCOMPARE(prima.codice, QString("RE_13"));
        QCOMPARE(prima.nome, QString("Alessandria-Pavia-Milano"));
        QCOMPARE(prima.direttrice, QString("D019"));
        QCOMPARE(prima.stato, TrenordVT::LineaRegolare);
        QCOMPARE(prima.descrizioneStato, QString("Circolazione Regolare"));

        // solo le linee non regolari vengono accodate per scaricarne gli avvisi
        const QStringList attese = {"RE_5", "RE_8", "R16", "R13", "R12", "S4", "S8", "S12", "S3", "S1", "S2", "S5"};
        const QQueue<QString> coda = parser.listaDirettrici();
        QCOMPARE(QStringList(coda.begin(), coda.end()), attese);

        for (const TrenordVT::Linea &linea : linee)
            if (attese.contains(linea.codice))
            {
                QCOMPARE(linea.stato, TrenordVT::LineaConCriticita);
                QCOMPARE(linea.descrizioneStato, QString::fromUtf8("Circolazione con criticità"));
            }
    }

    void listaLineeStati()
    {
        const QString html = lineaSintetica("S1", "green-line", "Circolazione Regolare")
                             + lineaSintetica("S2", "critical", "Circolazione con criticit&agrave;")
                             + lineaSintetica("S3", "critical", "Circolazione con gravi criticit&#224;")
                             // stessa linea ripetuta in un altro gruppo: va ignorata
                             + lineaSintetica("S2", "critical", "Circolazione con criticità");

        ParserTrenord parser(nullptr);
        QVERIFY(parser.analizzaListaDirettrici(comeJson(html)));
        QCOMPARE(parser.linee().size(), 3);
        QCOMPARE(parser.linee().at(1).stato, TrenordVT::LineaConCriticita);
        QCOMPARE(parser.linee().at(2).stato, TrenordVT::LineaConGraviCriticita);
        QCOMPARE(parser.linee().at(2).descrizioneStato, QString::fromUtf8("Circolazione con gravi criticità"));
        const QQueue<QString> coda = parser.listaDirettrici();
        QCOMPARE(QStringList(coda.begin(), coda.end()), QStringList({"S2", "S3"}));

        // accetta anche il frammento HTML non incapsulato in JSON
        ParserTrenord parser2(nullptr);
        QVERIFY(parser2.analizzaListaDirettrici(html));
        QCOMPARE(parser2.linee().size(), 3);
    }

    void listaLineeNonValida_data()
    {
        QTest::addColumn<QString>("risposta");
        QTest::newRow("vuota") << QString();
        QTest::newRow("403") << QStringLiteral("{\"code\":\"403\",\"message\":\"Forbidden\"}");
        QTest::newRow("json non valido") << QStringLiteral("{\"message\": ");
        QTest::newRow("pagina html") << QStringLiteral("<html><body><a href=\"/x\">x</a></body></html>");
        QTest::newRow("access denied") << leggiFixture("access_denied.html");
    }

    void listaLineeNonValida()
    {
        QFETCH(QString, risposta);

        ParserTrenord parser(nullptr);
        QVERIFY(parser.analizzaListaDirettrici(comeJson(lineaSintetica("S9", "critical", "x"))));
        QVERIFY(!parser.analizzaListaDirettrici(risposta));
        // un'analisi fallita non altera il risultato precedente
        QCOMPARE(parser.linee().size(), 1);
        QCOMPARE(parser.listaDirettrici().size(), 1);
    }

    void dettagliLineaReale()
    {
        QList<AvvisoTrenord> avvisi;
        QVERIFY(ParserTrenord::analizzaDettagliLinea(leggiFixture("trenord_dettagli_S11.json"),
                                                     "Como-Milano-Rho (S11)", avvisi));
        QCOMPARE(avvisi.size(), 5);

        const AvvisoTrenord primo = avvisi.at(0);
        QCOMPARE(primo.gravita(), TrenordVT::AvvisoCritico);
        QCOMPARE(primo.stato(), QString::fromUtf8("Criticità"));
        QCOMPARE(primo.direttrice(), QString("Como-Milano-Rho (S11)"));
        QCOMPARE(primo.orario(), QDateTime(QDate(2026, 10, 7), QTime(4, 56), QTimeZone::UTC).toLocalTime());
        QVERIFY(primo.testo().startsWith("LINEA S11\n"));
        QVERIFY(primo.testo().contains("guasto agli impianti di circolazione nella stazione di Milano Certosa"));
        QVERIFY(primo.testo().contains("- 25219 (COMO S.GIOVANNI 05:45 - RHO 07:17) termina a MILANO PORTA GARIBALDI"));
        // niente righe vuote ripetute né spazi non separabili
        QVERIFY(!primo.testo().contains("\n\n\n"));
        QVERIFY(!primo.testo().contains(QChar::Nbsp));
        // il sommario salta l'intestazione "LINEA S11"
        QVERIFY(primo.sommario().startsWith("Per le ripercussioni di un guasto"));

        QCOMPARE(avvisi.at(1).gravita(), TrenordVT::AvvisoInformativo);
        QCOMPARE(avvisi.at(1).stato(), QString("Regolare"));
        QCOMPARE(avvisi.at(4).gravita(), TrenordVT::AvvisoAttenzione);
        for (const AvvisoTrenord &a : avvisi)
        {
            QVERIFY(a.orario().isValid());
            QVERIFY(!a.testo().isEmpty());
        }
    }

    void caratteriWindows1252()
    {
        // virgolette "tipografiche" pubblicate come caratteri di controllo C1
        const QString html = QStringLiteral("<div class=\"specific-line\"><div class=\"item 1 info \">"
                                            "<span class=\"news-date\">2026-10-02T08:15:10.000Z</span>"
                                            "<div class=\"col-xs-12 body-texts no-margin\"><p>della ")
                             + QChar(0x93) + QStringLiteral("fiera di Delebio") + QChar(0x94) + QChar(0x81)
                             + QStringLiteral(" </p></div></div></div>");
        QList<AvvisoTrenord> avvisi;
        QVERIFY(ParserTrenord::analizzaDettagliLinea(comeJson(html), "R13", avvisi));
        QCOMPARE(avvisi.size(), 1);
        QCOMPARE(avvisi.at(0).testo(), QString::fromUtf8("della “fiera di Delebio”"));
    }

    void dettagliLineaNonValida()
    {
        QList<AvvisoTrenord> avvisi;
        QVERIFY(!ParserTrenord::analizzaDettagliLinea(QString(), "x", avvisi));
        QVERIFY(!ParserTrenord::analizzaDettagliLinea(QStringLiteral("{\"code\":\"403\",\"message\":\"Forbidden\"}"), "x", avvisi));
        QVERIFY(avvisi.isEmpty());

        // scheda di linea senza avvisi
        QVERIFY(ParserTrenord::analizzaDettagliLinea(comeJson("<div class=\"specific-line\"></div>"), "x", avvisi));
        QVERIFY(avvisi.isEmpty());
    }

    void unisciAvvisi()
    {
        const QDateTime t1(QDate(2026, 10, 7), QTime(8, 0));
        const QDateTime t2(QDate(2026, 10, 7), QTime(9, 0));

        auto crea = [](const QString &linea, const QDateTime &orario, TrenordVT::GravitaAvviso g, const QString &testo) {
            AvvisoTrenord a;
            a.impostaDirettrice(linea);
            a.impostaOrario(orario);
            a.impostaGravita(g);
            a.impostaTesto(testo);
            return a;
        };

        const QList<AvvisoTrenord> uniti = ParserTrenord::unisciAvvisi({
            crea("S1", t1, TrenordVT::AvvisoInformativo, "lavori"),
            crea("S2", t1, TrenordVT::AvvisoInformativo, "lavori"),
            crea("S2", t2, TrenordVT::AvvisoInformativo, "recente"),
            crea("S3", t1, TrenordVT::AvvisoCritico, "guasto"),
            crea("S1", t1, TrenordVT::AvvisoInformativo, "lavori"),
        });

        QCOMPARE(uniti.size(), 3);
        // prima il più grave, poi i più recenti
        QCOMPARE(uniti.at(0).testo(), QString("guasto"));
        QCOMPARE(uniti.at(1).testo(), QString("recente"));
        QCOMPARE(uniti.at(2).direttrice(), QString("S1, S2"));
    }

    void modello()
    {
        QList<AvvisoTrenord> avvisi;
        QVERIFY(ParserTrenord::analizzaDettagliLinea(leggiFixture("trenord_dettagli_S11.json"), "S11", avvisi));

        ModelloAvvisiTrenord modello(nullptr);
        QCOMPARE(modello.rowCount(QModelIndex()), 0);
        QCOMPARE(modello.columnCount(QModelIndex()), int(ModelloAvvisiTrenord::numeroColonne));

        modello.impostaAvvisi(ParserTrenord::unisciAvvisi(avvisi));
        QCOMPARE(modello.rowCount(QModelIndex()), 5);
        QCOMPARE(modello.headerData(ModelloAvvisiTrenord::colLinea, Qt::Horizontal).toString(), QString("Linea"));

        const QModelIndex avviso = modello.index(0, ModelloAvvisiTrenord::colAvviso);
        QVERIFY(avviso.data().toString().startsWith("Per le ripercussioni"));
        QVERIFY(avviso.data(Qt::ToolTipRole).toString().contains("Milano Certosa"));
        QVERIFY(avviso.data(Qt::BackgroundRole).isValid());
        QCOMPARE(modello.index(0, ModelloAvvisiTrenord::colLinea).data().toString(), QString("S11"));
        QCOMPARE(modello.index(0, ModelloAvvisiTrenord::colStato).data().toString(), QString::fromUtf8("Criticità"));
        QVERIFY(!modello.index(0, ModelloAvvisiTrenord::colOrario).data().toString().isEmpty());
        // gli avvisi informativi non hanno sfondo
        QVERIFY(!modello.index(4, 0).data(Qt::BackgroundRole).isValid());
        QVERIFY(!modello.index(99, 0).data().isValid());
    }
};

QTEST_MAIN(TstParserTrenord)
#include "tst_parsertrenord.moc"
