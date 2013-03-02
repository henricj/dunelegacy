
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

int main(int argc, char** argv) {
  CppUnit::TextUi::TestRunner testrunner;
  CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
  testrunner.addTest(registry.makeTest());
  return !testrunner.run("", false, true, false);
}

