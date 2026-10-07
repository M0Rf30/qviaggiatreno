// Test dei parser ViaggiaTreno (stazione e treno) con pagine di esempio scritte a mano
// in tests/data, che documentano la struttura DOM attesa dai parser
#include "parser_viaggiatreno_base.h"
#include "parser_viaggiatreno_stazione.h"
#include "parser_viaggiatreno_treno.h"

#include <QFile>
#include <QtTest>

namespace {

// riproduce DownloadViaggiaTreno::correggiOutputVT (privata nell'applicazione): i parser
// ricevono il testo gia' trattato in questo modo
QString correggiOutputVT(QString testo)
{
    testo = testo.simplified();
    testo.replace(QStringLiteral("&#039;"), QStringLiteral("'"));
    testo.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
    testo.replace(QStringLiteral("<br>"), QStringLiteral("<br/>"));
    testo.replace(QStringLiteral("</strong> <br/> <br/>"), QStringLiteral("</strong> </p> <br/>"));
    return testo;
}

QString leggiFixture(const QString &nome)
{
    QFile file(QStringLiteral(QVT_TEST_DATA "/") + nome);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    return correggiOutputVT(QString::fromUtf8(file.readAll()));
}

} // namespace

class TstParserViaggiaTreno : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    // --- sostituzione nomi stazione ---------------------------------------------------------
    void sostituisciNomeStazione()
    {
        QCOMPARE(ParserViaggiaTrenoBase::sostituisciNomeStazione("M N CADORNA"), QString("MILANO NORD CADORNA"));
        QCOMPARE(ParserViaggiaTrenoBase::sostituisciNomeStazione("CAMNAGO LENTATE"), QString("CAMNAGO-LENTATE"));
        QCOMPARE(ParserViaggiaTrenoBase::sostituisciNomeStazione("MONZA"), QString("MONZA"));
    }

    // --- stazione ------------------------------------------------------------------------------
    void stazione()
    {
        const QString pagina = leggiFixture("stazione_arrivi_partenze.html");
        QVERIFY(!pagina.isEmpty());

        ParserStazioneViaggiaTreno parser(nullptr);
        parser.impostaRispostaVT(pagina);
        QVERIFY(!parser.stazioneNonTrovata());
        QVERIFY(!parser.nomeStazioneAmbiguo());
        QVERIFY2(parser.analizza(), qPrintable(parser.errore()));
        QCOMPARE(parser.stazione(), QString("MILANO CENTRALE"));

        // il terzo treno non ha codice di origine e viene scartato
        const QList<StazioneVT::DatiTreno> partenze = parser.partenze();
        QCOMPARE(partenze.size(), 2);

        QCOMPARE(partenze.at(0).categoria(), QString("REG"));
        QCOMPARE(partenze.at(0).numero(), QString("2345"));
        QCOMPARE(partenze.at(0).stazione(), QString("BRESCIA"));
        QCOMPARE(partenze.at(0).orario(), QString("10:15"));
        QCOMPARE(partenze.at(0).binarioProgrammato(), QString("5"));
        QCOMPARE(partenze.at(0).binarioReale(), QString("5"));
        QCOMPARE(partenze.at(0).ritardo(), QString("0"));
        QCOMPARE(partenze.at(0).codiceOrigine(), QString("S01700"));

        QCOMPARE(partenze.at(1).categoria(), QString("IC"));
        QCOMPARE(partenze.at(1).numero(), QString("601"));
        QCOMPARE(partenze.at(1).stazione(), QString("VENEZIA S.LUCIA"));
        QCOMPARE(partenze.at(1).orario(), QString("11:00"));
        QCOMPARE(partenze.at(1).binarioProgrammato(), QString("7"));
        QCOMPARE(partenze.at(1).binarioReale(), QString("8"));
        QCOMPARE(partenze.at(1).ritardo(), QString("+12"));

        const QList<StazioneVT::DatiTreno> arrivi = parser.arrivi();
        QCOMPARE(arrivi.size(), 2);

        QCOMPARE(arrivi.at(0).categoria(), QString("R"));
        QCOMPARE(arrivi.at(0).numero(), QString("2400"));
        QCOMPARE(arrivi.at(0).stazione(), QString("VARESE"));
        QCOMPARE(arrivi.at(0).orario(), QString("10:45"));
        QCOMPARE(arrivi.at(0).binarioProgrammato(), QString("3 A"));
        QCOMPARE(arrivi.at(0).binarioReale(), QString("3 A"));
        QCOMPARE(arrivi.at(0).ritardo(), QString("+7"));
        QCOMPARE(arrivi.at(0).codiceOrigine(), QString("S01322"));

        QCOMPARE(arrivi.at(1).numero(), QString("9701"));
        QCOMPARE(arrivi.at(1).binarioProgrammato(), QString("Sconosciuto"));
        QCOMPARE(arrivi.at(1).binarioReale(), QString("Sconosciuto"));
        QCOMPARE(arrivi.at(1).ritardo(), QString("0"));
        QCOMPARE(arrivi.at(1).codiceOrigine(), QString("S08409"));

        // il modello espone gli stessi dati
        ModelloStazione modello(nullptr, true);
        modello.aggiornaModello(partenze);
        QCOMPARE(modello.rowCount(QModelIndex()), 2);
        QCOMPARE(modello.columnCount(QModelIndex()), int(StazioneVT::colUltima) + 1);
        QCOMPARE(modello.data(modello.index(0, StazioneVT::colNumero), Qt::DisplayRole).toString(), QString("2345"));
        QCOMPARE(modello.data(modello.index(0, StazioneVT::colRitardo), Qt::DisplayRole).toString(), QString("In orario"));
        QCOMPARE(modello.data(modello.index(1, StazioneVT::colRitardo), Qt::DisplayRole).toString(), QString("12 minuti"));
        QCOMPARE(modello.categorie(), (QStringList{"IC", "REG"}));
    }

    void stazioneNonTrovata()
    {
        ParserStazioneViaggiaTreno parser(nullptr);
        parser.impostaRispostaVT(correggiOutputVT("<html><body><p>Localita' non trovata</p></body></html>"));
        QVERIFY(parser.stazioneNonTrovata());
        parser.impostaRispostaVT(correggiOutputVT("<html><body><p>Inserire almeno 2 caratteri</p></body></html>"));
        QVERIFY(parser.stazioneNonTrovata());
    }

    void stazioneAmbigua()
    {
        const QString pagina = leggiFixture("stazione_ambigua.html");
        QVERIFY(!pagina.isEmpty());

        ParserStazioneViaggiaTreno parser(nullptr);
        parser.impostaRispostaVT(pagina);
        QVERIFY(parser.nomeStazioneAmbiguo());

        const QMap<QString, QString> codici = parser.listaCodiciStazioni(pagina);
        QCOMPARE(codici.size(), 2);
        QCOMPARE(codici.value("MONZA"), QString("S01700"));
        QCOMPARE(codici.value("MONZA SOBBORGHI"), QString("S01701"));
    }

    // --- treno -----------------------------------------------------------------------------------
    void trenoRiepilogo()
    {
        const QString pagina = leggiFixture("treno_riepilogo.html");
        QVERIFY(!pagina.isEmpty());

        ParserTrenoViaggiaTreno parser(nullptr);
        parser.impostaRispostaVTRiepilogo(pagina);
        QVERIFY(!parser.trenoNonPrevisto());
        QVERIFY(!parser.numeroTrenoAmbiguo());
        QVERIFY(!parser.trenoSoppressoTotalmente());

        TrenoVT::DatiTreno treno;
        QVERIFY2(parser.analizzaRiepilogo(treno), qPrintable(parser.errore()));

        QCOMPARE(treno.numeroTreno(), QString("REG 2345"));
        QCOMPARE(treno.categoriaTreno(), QString("REG"));
        QCOMPARE(treno.statoTreno(), TrenoVT::TrenoInViaggio);
        QCOMPARE(treno.dato(TrenoVT::dtProvvedimenti), QString("Sciopero in corso"));

        QCOMPARE(treno.dato(TrenoVT::dtPartenza), QString("Partenza: MILANO CENTRALE"));
        QCOMPARE(treno.dato(TrenoVT::dtOrarioPartenzaProgrammato), QString("10:15"));
        QCOMPARE(treno.dato(TrenoVT::dtOrarioPartenzaReale), QString("10:17"));
        QCOMPARE(treno.dato(TrenoVT::dtBinarioPartenzaProgrammato), QString("5"));
        QCOMPARE(treno.dato(TrenoVT::dtBinarioPartenzaReale), QString("5"));

        QCOMPARE(treno.dato(TrenoVT::dtArrivo), QString("Arrivo: BRESCIA"));
        QCOMPARE(treno.dato(TrenoVT::dtOrarioArrivoProgrammato), QString("11:30"));
        QCOMPARE(treno.dato(TrenoVT::dtOrarioArrivo), QString("11:35"));
        QCOMPARE(treno.dato(TrenoVT::dtBinarioArrivoProgrammato), QString("5"));
        QCOMPARE(treno.dato(TrenoVT::dtBinarioArrivoReale), QString("6"));

        QCOMPARE(treno.dato(TrenoVT::dtRitardoTransito), QString("5 minuti in ritardo"));
        QCOMPARE(treno.dato(TrenoVT::dtOrarioTransito), QString("10:25"));
        QCOMPARE(treno.dato(TrenoVT::dtUltimoRilevamento), QString("ROGOREDO"));
    }

    void trenoRiepilogoPerLista()
    {
        const QString pagina = leggiFixture("treno_riepilogo.html");
        QVERIFY(!pagina.isEmpty());

        ParserTrenoViaggiaTreno parser(nullptr);
        parser.impostaRispostaVTRiepilogo(pagina);

        ListaVT::DatiTreno treno("2345");
        QVERIFY2(parser.analizzaRiepilogoPerLista(treno), qPrintable(parser.errore()));

        QCOMPARE(treno.statoTreno(), TrenoVT::TrenoInViaggio);
        QCOMPARE(treno.dato(ListaVT::dtCategoria), QString("REG"));
        QCOMPARE(treno.dato(ListaVT::dtOrigine), QString("Partenza: MILANO CENTRALE"));
        QCOMPARE(treno.dato(ListaVT::dtPartenzaProgrammata), QString("10:15"));
        QCOMPARE(treno.dato(ListaVT::dtPartenzaEffettiva), QString("10:17"));
        QCOMPARE(treno.dato(ListaVT::dtDestinazione), QString("Arrivo: BRESCIA"));
        QCOMPARE(treno.dato(ListaVT::dtArrivoProgrammato), QString("11:30"));
        QCOMPARE(treno.dato(ListaVT::dtUltimaFermata), QString("ROGOREDO"));
        QCOMPARE(treno.dato(ListaVT::dtOrarioFermataProgrammato), QString("10:24"));
        QCOMPARE(treno.dato(ListaVT::dtOrarioFermataEffettivo), QString("10:25"));
        QCOMPARE(treno.dato(ListaVT::dtRitardoTransito), QString("5 minuti in ritardo"));
        QCOMPARE(treno.dato(ListaVT::dtOrarioTransito), QString("10:25"));
        QCOMPARE(treno.dato(ListaVT::dtUltimoRilevamento), QString("ROGOREDO"));
    }

    void trenoDettagli()
    {
        const QString pagina = leggiFixture("treno_dettagli.html");
        QVERIFY(!pagina.isEmpty());

        ParserTrenoViaggiaTreno parser(nullptr);
        parser.impostaRispostaVTDettagli(pagina);

        TrenoVT::DatiTreno treno;
        QVERIFY2(parser.analizzaDettagli(treno), qPrintable(parser.errore()));

        const QList<TrenoVT::Fermata *> fermate = treno.fermate();
        QCOMPARE(fermate.size(), 4);

        QCOMPARE(fermate.at(0)->nomeFermata(), QString("MONZA"));
        QVERIFY(fermate.at(0)->effettuata());
        QVERIFY(!fermate.at(0)->soppressa());
        QCOMPARE(fermate.at(0)->oraArrivoProgrammata(), QString("10:30"));
        QCOMPARE(fermate.at(0)->oraArrivoReale(), QString("10:31"));
        QCOMPARE(fermate.at(0)->binarioProgrammato(), QString("3"));
        QCOMPARE(fermate.at(0)->binarioReale(), QString("4"));

        QCOMPARE(fermate.at(1)->nomeFermata(), QString("LECCO"));
        QVERIFY(!fermate.at(1)->effettuata());
        QCOMPARE(fermate.at(1)->oraArrivoProgrammata(), QString("10:50"));
        QCOMPARE(fermate.at(1)->oraArrivoStimata(), QString("10:55"));
        QCOMPARE(fermate.at(1)->binarioProgrammato(), QString("5"));
        QCOMPARE(fermate.at(1)->binarioReale(), QString("6"));

        QCOMPARE(fermate.at(2)->nomeFermata(), QString("COLICO"));
        QVERIFY(fermate.at(2)->soppressa());
        QVERIFY(!fermate.at(2)->effettuata());
        QCOMPARE(fermate.at(2)->binarioProgrammato(), QString("7"));
        QCOMPARE(fermate.at(2)->binarioReale(), QString("8"));

        QCOMPARE(fermate.at(3)->nomeFermata(), QString("SONDRIO"));
        QCOMPARE(fermate.at(3)->binarioProgrammato(), QString("9"));
        QCOMPARE(fermate.at(3)->binarioReale(), QString("--"));

        // le fermate sono condivise tra le copie e restano valide dopo la distruzione dell'originale
        auto copia = std::make_unique<TrenoVT::DatiTreno>(treno);
        QCOMPARE(copia->fermate().size(), 4);
    }

    void trenoMessaggi()
    {
        ParserTrenoViaggiaTreno parser(nullptr);

        parser.impostaRispostaVTRiepilogo(correggiOutputVT("<html><body><p>Numero treno non valido</p></body></html>"));
        QVERIFY(parser.trenoNonPrevisto());

        parser.impostaRispostaVTRiepilogo(correggiOutputVT(
            "<html><body><form><select><option>518 - SARONNO</option></select></form></body></html>"));
        QVERIFY(parser.numeroTrenoAmbiguo());

        parser.impostaRispostaVTRiepilogo(correggiOutputVT(
            "<html><body><span class=\"errore\">Il treno e' stato cancellato</span></body></html>"));
        QVERIFY(parser.trenoSoppressoTotalmente());

        const QString ambigua = correggiOutputVT(
            "<html><body><div>x</div><div><form><p><select>"
            "<option value=\"518;S09218\">518 - NAPOLI CENTRALE</option>"
            "<option value=\"518;S01322\">518 - SARONNO</option>"
            "</select></p></form></div></body></html>");
        const QMap<QString, QString> codici = parser.listaCodiciTreno(ambigua);
        QCOMPARE(codici.size(), 2);
        QCOMPARE(codici.value("518 - SARONNO"), QString("518;S01322"));
    }

    // --- input malformato: nessun crash ----------------------------------------------------------
    void inputMalformato_data()
    {
        QTest::addColumn<QString>("pagina");
        QTest::newRow("vuoto") << QString();
        QTest::newRow("non xml") << correggiOutputVT("questo non e' xml");
        QTest::newRow("access denied") << leggiFixture("access_denied.html");
        QTest::newRow("html senza body") << correggiOutputVT("<html></html>");
        QTest::newRow("body vuoto") << correggiOutputVT("<html><body></body></html>");
        QTest::newRow("troncato") << correggiOutputVT("<html><body><h1>REG 1</h1><div><h2>Partenza");
        QTest::newRow("commenti senza treni")
            << correggiOutputVT("<html><body><!-- partenze --><!-- arrivi --><!-- fine --></body></html>");
        QTest::newRow("treno senza dati")
            << correggiOutputVT("<html><body><!-- partenze --><div/><!-- arrivi --><div><h2>X</h2></div><!-- fine --></body></html>");
    }

    void inputMalformato()
    {
        QFETCH(QString, pagina);

        // stazione: deve fallire oppure produrre liste vuote
        ParserStazioneViaggiaTreno stazione(nullptr);
        stazione.impostaRispostaVT(pagina);
        stazione.stazioneNonTrovata();
        stazione.nomeStazioneAmbiguo();
        const bool okStazione = stazione.analizza();
        if (okStazione)
        {
            QVERIFY(stazione.partenze().isEmpty());
            QVERIFY(stazione.arrivi().isEmpty());
        }
        else
        {
            QVERIFY(stazione.rigaErrore() >= -1);
        }
        QVERIFY(stazione.listaCodiciStazioni(pagina).isEmpty());

        // treno: nessuna delle analisi deve andare in crash
        ParserTrenoViaggiaTreno treno(nullptr);
        treno.impostaRispostaVTRiepilogo(pagina);
        treno.impostaRispostaVTDettagli(pagina);
        TrenoVT::DatiTreno dati;
        treno.analizzaRiepilogo(dati);
        treno.analizzaDettagli(dati);
        ListaVT::DatiTreno lista("1");
        treno.analizzaRiepilogoPerLista(lista);
        QVERIFY(treno.listaCodiciTreno(pagina).isEmpty());
    }

    void erroreXmlRiportato()
    {
        ParserStazioneViaggiaTreno stazione(nullptr);
        stazione.impostaRispostaVT(QStringLiteral("<html><body></html>"));
        QVERIFY(!stazione.analizza());
        QVERIFY(!stazione.errore().isEmpty());
        QVERIFY(stazione.rigaErrore() >= 1);

        ParserTrenoViaggiaTreno treno(nullptr);
        treno.impostaRispostaVTRiepilogo(QStringLiteral("<html><body></html>"));
        TrenoVT::DatiTreno dati;
        QVERIFY(!treno.analizzaRiepilogo(dati));
        QVERIFY(!treno.errore().isEmpty());
        QVERIFY(treno.rigaErrore() >= 1);
    }

    void paginaSenzaDatiTreno()
    {
        // pagina ben formata ma senza i div attesi: il riepilogo segnala l'errore invece di andare in crash
        ParserTrenoViaggiaTreno treno(nullptr);
        treno.impostaRispostaVTRiepilogo(QStringLiteral("<html><body><p>x</p></body></html>"));
        TrenoVT::DatiTreno dati;
        QVERIFY(!treno.analizzaRiepilogo(dati));
    }
};

QTEST_MAIN(TstParserViaggiaTreno)
#include "tst_parserviaggiatreno.moc"
