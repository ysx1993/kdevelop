#include <QtTest/QtTest>

#include "ast.h"
#include "parser.h"
#include "rpp/preprocessor.h"
#include "control.h"
#include "dumptree.h"
#include "tokens.h"
#include "parsesession.h"
#include "commentformatter.h"
#include "codegenerator.h"

#include "testconfig.h"

#include <QByteArray>
#include <QDataStream>
#include <QFile>

#include <iostream>
#include <rpp/chartools.h>
#include <rpp/pp-engine.h>

class TestGenerator : public QObject
{
  Q_OBJECT

  Control control;
  DumpTree dumper;

public:
  TestGenerator()
  {
  }

private slots:
  void initTestCase()
  {
  }

  void testIf()
  {
    QByteArray method("void test() { if (i == 0) { foo(); } else { foo2(); } }");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    //dumper.dump(ast, lastSession->token_stream);

    CodeGenerator cg(lastSession);
    cg.visit(ast);
    kDebug() << method;
    kDebug() << cg.output();
  }

  void testFor()
  {
    QByteArray method("void test() { for (int i = 0; i < 4; ++i) { break; } for (j; j < 4; ) {return;} }");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    //dumper.dump(ast, lastSession->token_stream);

    CodeGenerator cg(lastSession);
    cg.visit(ast);
    kDebug() << method;
    kDebug() << cg.output();
  }

  void testDo()
  {
    QByteArray method("void test() { do { foo(); } while (i < 0); }");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    //dumper.dump(ast, lastSession->token_stream);

    CodeGenerator cg(lastSession);
    cg.visit(ast);
    kDebug() << method;
    kDebug() << cg.output();
  }

  void testWhile()
  {
    QByteArray method("void test() { while (i & 3) { foo(); } }");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    //dumper.dump(ast, lastSession->token_stream);

    CodeGenerator cg(lastSession);
    cg.visit(ast);
    kDebug() << method;
    kDebug() << cg.output();
  }

  void testSwitch()
  {
    QByteArray method("void test() { switch (i) { case 1: break; case 2: return; default: goto foo; } foo: return; }");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    //dumper.dump(ast, lastSession->token_stream);

    CodeGenerator cg(lastSession);
    cg.visit(ast);
    kDebug() << method;
    kDebug() << cg.output();
  }

  void testClass()
  {
    QByteArray method("struct A : public B, virtual private C { int i; A() : i(5) { } virtual void test() = 0; };");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    //dumper.dump(ast, lastSession->token_stream);

    CodeGenerator cg(lastSession);
    cg.visit(ast);
    kDebug() << method;
    kDebug() << cg.output();
  }

  void testTemplateClass()
  {
    QByteArray method("template <typename B> struct A : private C { B i; A() : i(5) { } virtual void test() = 0; };");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    //dumper.dump(ast, lastSession->token_stream);

    CodeGenerator cg(lastSession);
    cg.visit(ast);
    kDebug() << method;
    kDebug() << cg.output();
  }

  void testMethod()
  {
    QByteArray method("int A::test(int primitive, B* pointer) { return primitive; }");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    //dumper.dump(ast, lastSession->token_stream);

    CodeGenerator cg(lastSession);
    cg.visit(ast);
    kDebug() << method;
    kDebug() << cg.output();
  }

private:
  ParseSession* lastSession;

  TranslationUnitAST* parse(const QByteArray& unit, pool* mem_pool)
  {
    Parser parser(&control);
    lastSession = new ParseSession();
    lastSession->setContentsAndGenerateLocationTable(tokenizeFromByteArray(unit));
    return  parser.parse(lastSession);
  }
};

#include "test_generator.moc"

QTEST_MAIN(TestGenerator)
