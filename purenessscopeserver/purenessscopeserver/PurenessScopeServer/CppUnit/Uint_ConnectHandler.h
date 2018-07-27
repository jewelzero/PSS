#ifndef UNIT_CONNECTHANDLER_H
#define UNIT_CONNECTHANDLER_H

#ifdef _CPPUNIT_TEST

#include "Unit_Common.h"
#include "ConnectHandler.h"

class CUnit_ConnectHandler : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CUnit_ConnectHandler);
    CPPUNIT_TEST(Test_ConnectHandler_Stream);
    CPPUNIT_TEST(Test_ConnectHandler_CloseMessages);
    CPPUNIT_TEST(Test_ConnectHandler_Debug);
    CPPUNIT_TEST_SUITE_END();

public:
    virtual ~CUnit_ConnectHandler();

    virtual void setUp(void);

    virtual void tearDown(void);

    void Test_ConnectHandler_Stream(void);

    void Test_ConnectHandler_CloseMessages(void);

    void Test_ConnectHandler_Debug(void);

private:
    CConnectHandler* m_pConnectHandler;
};

#endif

#endif