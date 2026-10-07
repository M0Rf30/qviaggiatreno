// Test di ParserTrenord::analizzaListaDirettrici con pagine HTML sintetiche
#include "parser_trenord.h"

#include <QStringList>
#include <QtTest>

class TstParserTrenord : public QObject
{
    Q_OBJECT

private:
    // esegue il parser e restituisce la coda come lista
    static QStringList analizza(const QString &html, bool *esito)
    {
        ParserTrenord parser(nullptr);
        *esito = parser.analizzaListaDirettrici(html);
        const QQueue<QString> coda = parser.listaDirettrici();
        return QStringList(coda.begin(), coda.end());
    }

private Q_SLOTS:
    void indicatoreAcceso()
    {
        bool ok;
        const QStringList l = analizza(
            "<ul><li class=\"direttrici-item\"><a href=\"/d1.aspx\">Linea 1</a>"
            "<img class=\"indicator\" src=\"/img/indicator-on.png\"/></li></ul>", &ok);
        QVERIFY(ok);
        QCOMPARE(l, QStringList{"/d1.aspx"});
    }

    void indicatoreSpento()
    {
        bool ok;
        const QStringList l = analizza(
            "<ul><li class=\"direttrici-item\"><a href=\"/d1.aspx\">Linea 1</a>"
            "<img class=\"indicator\" src=\"/img/indicator-off.png\"/></li></ul>", &ok);
        QVERIFY(ok);
        QVERIFY(l.isEmpty());
    }

    void immagineAssente()
    {
        bool ok;
        const QStringList l = analizza(
            "<ul><li class=\"direttrici-item\"><a href=\"/d1.aspx\">Linea 1</a></li></ul>", &ok);
        QVERIFY(ok);
        QVERIFY(l.isEmpty());
    }

    void immagineSenzaClasseIndicator()
    {
        bool ok;
        const QStringList l = analizza(
            "<ul><li class=\"direttrici-item\"><a href=\"/d1.aspx\">x</a>"
            "<img class=\"logo\" src=\"indicator-on.png\"/></li></ul>", &ok);
        QVERIFY(ok);
        QVERIFY(l.isEmpty());
    }

    void primaImmagineIndicatorDecide()
    {
        bool ok;
        const QStringList l = analizza(
            "<li class=\"direttrici-item\"><a href=\"/d1\">x</a>"
            "<img class=\"indicator\" src=\"indicator-off.png\">"
            "<img class=\"indicator\" src=\"indicator-on.png\"></li>", &ok);
        QVERIFY(ok);
        QVERIFY(l.isEmpty());
    }

    void classiMultiple()
    {
        bool ok;
        const QStringList l = analizza(
            "<li class=\"foo direttrici-item  bar\"><a class=\"x\" href=\"/d2\">x</a>"
            "<img class=\"big indicator\" src=\"a/indicator-on.png\"></li>"
            "<li class=\"direttrici-item-altro\"><a href=\"/no\">x</a>"
            "<img class=\"indicator\" src=\"indicator-on.png\"></li>", &ok);
        QVERIFY(ok);
        QCOMPARE(l, QStringList{"/d2"});
    }

    void ordineAttributiApiciMaiuscole()
    {
        bool ok;
        const QStringList l = analizza(
            "<LI ID='a' CLASS='direttrici-item'>\n"
            "  <A TITLE=\"t\"\n   HREF='/d3.aspx?x=1&amp;y=2'>Linea</A>\n"
            "  <IMG SRC='/i/indicator-on.gif' ALT=\"a > b\" CLASS=indicator />\n"
            "</LI>", &ok);
        QVERIFY(ok);
        QCOMPARE(l, QStringList{"/d3.aspx?x=1&y=2"});
    }

    void srcPrimaDiClass()
    {
        bool ok;
        const QStringList l = analizza(
            "<li class=\"direttrici-item\"><a href=\"/d4\">x</a>"
            "<img src=\"indicator-on.png\" alt=\"\" class=\"indicator\"></li>", &ok);
        QVERIFY(ok);
        QCOMPARE(l, QStringList{"/d4"});
    }

    void nessunaLista_data()
    {
        QTest::addColumn<QString>("html");
        QTest::newRow("vuoto") << QString();
        QTest::newRow("testo") << QStringLiteral("non e' html");
        QTest::newRow("altro li") << QStringLiteral("<ul><li class=\"altro\"><a href=\"/x\">x</a></li></ul>");
        QTest::newRow("access denied")
            << QStringLiteral("<HTML><HEAD><TITLE>Access Denied</TITLE></HEAD><BODY><H1>Access Denied</H1></BODY></HTML>");
    }

    void nessunaLista()
    {
        QFETCH(QString, html);
        bool ok = true;
        const QStringList l = analizza(html, &ok);
        QVERIFY(!ok);
        QVERIFY(l.isEmpty());
    }

    void piuElementiOrdineCoda()
    {
        bool ok;
        const QStringList l = analizza(
            "<ul>"
            "<li class=\"direttrici-item\"><a href=\"/uno\">1</a><img class=\"indicator\" src=\"indicator-on.png\"></li>"
            "<li class=\"direttrici-item\"><a href=\"/due\">2</a><img class=\"indicator\" src=\"indicator-off.png\"></li>"
            "<li class=\"direttrici-item\"><a href=\"/tre\">3</a><img class=\"indicator\" src=\"indicator-on.png\"></li>"
            "<li class=\"direttrici-item\"><a href=\"/quattro\">4</a><img class=\"indicator\" src=\"indicator-on.png\"></li>"
            "</ul>", &ok);
        QVERIFY(ok);
        QCOMPARE(l, (QStringList{"/uno", "/tre", "/quattro"}));
    }

    void secondaAnalisiSostituisceCoda()
    {
        ParserTrenord parser(nullptr);
        QVERIFY(parser.analizzaListaDirettrici(
            "<li class=\"direttrici-item\"><a href=\"/uno\">1</a><img class=\"indicator\" src=\"indicator-on.png\"></li>"));
        QCOMPARE(parser.listaDirettrici().size(), 1);
        // una pagina non valida non modifica la coda precedente
        QVERIFY(!parser.analizzaListaDirettrici("<html></html>"));
        QCOMPARE(parser.listaDirettrici().size(), 1);
        QVERIFY(parser.analizzaListaDirettrici(
            "<li class=\"direttrici-item\"><a href=\"/due\">2</a><img class=\"indicator\" src=\"indicator-off.png\"></li>"));
        QVERIFY(parser.listaDirettrici().isEmpty());
    }
};

QTEST_MAIN(TstParserTrenord)
#include "tst_parsertrenord.moc"
