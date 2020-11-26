// LogManager.h
// Log的管理类，负责Log日志的对象管理。
// 一开始想把它作为一个DLL单独实现，但是考虑一下，还是放在框架里面实现比较好。
// add by freeeyes
// 2009-04-04

#include "LogManager.h"

int CLogManager::open()
{
    m_blRun = true;

    //开启一个线程队列处理
    std::thread tt_dispose([&]()
        {
            svc();
        });

    tt_dispose.detach();

    return 0;
}

int CLogManager::svc(void)
{
    OUR_DEBUG((LM_INFO,"[CLogManager::svc] svc run.\n"));

    while(m_blRun)
    {
        shared_ptr<_LogBlockInfo> msg;
        if (true == m_objThreadQueue.Pop(msg))
        {
            //消息处理
            Dispose_Queue(msg);
        }
        else
        {
            //消息接收失败
            break;
        }

    }

    //回收日志对象
    m_pServerLogger->Close();

    OUR_DEBUG((LM_ERROR, "[CLogManager::svc]Close OK.\n"));
    return 0;
}

int CLogManager::Close()
{
    if(true == m_blRun)
    {
        m_blRun = false;
    }

    return 0;
}

void CLogManager::Init(int nThreadCount, int nQueueMax, uint32 u4MailID)
{
    m_nThreadCount = nThreadCount;
    m_nQueueMax    = nQueueMax;

    if(u4MailID > 0)
    {
        m_blIsMail = true;
    }
}

int CLogManager::Start()
{
    if(m_blRun == true)
    {
        return 0;
    }
    else
    {
        SetReset(false);
        return open();
    }
}

int CLogManager::Stop()
{
    Close();
    return 0;
}

bool CLogManager::IsRun() const
{
    return m_blRun;
}

int CLogManager::PutLog(shared_ptr<_LogBlockInfo> pLogBlockInfo)
{
    //如果正在重新加载
    if(m_blIsNeedReset == true)
    {
        return 0;
    }

    if ((int)m_objThreadQueue.Size() >= m_nQueueMax)
    {
        OUR_DEBUG((LM_INFO,"[CLogManager::PutLog] CLogManager queue is full!\n"));
        return -1;
    }

    ACE_Time_Value xtime = ACE_OS::gettimeofday()+ACE_Time_Value(0, MAX_MSG_PUTTIMEOUT);
    m_objThreadQueue.Push(pLogBlockInfo);

    return 0;
}

int CLogManager::RegisterLog(shared_ptr<IServerLogger> pServerLogger)
{
    m_pServerLogger = pServerLogger;
    return 0;
}

int CLogManager::UnRegisterLog()
{
    if(m_pServerLogger != nullptr)
    {
        m_pServerLogger->Close();
    }

    return 0;
}

bool CLogManager::Dispose_Queue(shared_ptr<_LogBlockInfo> msg)
{
    if (0 != ProcessLog(msg))
    {
        OUR_DEBUG((LM_ERROR, "[CLogManager::svc] ProcessLog is false.\n"));
    }

    return true;
}

int CLogManager::ProcessLog(shared_ptr<_LogBlockInfo> msg)
{
    if(nullptr == m_pServerLogger)
    {
        return -1;
    }

    m_pServerLogger->DoLog(msg->m_u2LogID, msg);
    return 0;
}

int CLogManager::WriteToMail_i(uint16 u2LogType, uint16 u2MailID, string strTitle, string strLog)
{
    auto msg = std::make_shared<_LogBlockInfo>();

    msg->m_u4Length     = (uint32)strLog.length();
    msg->m_strBlock     = strLog;
    msg->m_strMailTitle = strTitle;
    msg->m_u2MailID     = u2MailID;
    msg->m_u2LogID      = u2LogType;

    PutLog(msg);
    return 0;
}

int CLogManager::WriteLogBinary(uint16 u2LogType, string strText)
{
    //将数据转换为二进制
    string strHex = buffer_to_Hex_string(strText);

    return  WriteLog_i(u2LogType, strHex);
}

int CLogManager::WriteLog_r(uint16 u2LogType, string strLog)
{
    return WriteLog_i(u2LogType, strLog);
}

int CLogManager::WriteToMail_r(uint16 u2LogType, uint16 u2MailID, string strTitle, string strLog)
{
    return WriteToMail_i(u2LogType, u2MailID, strTitle, strLog);
}

//*****************************************************************************


void CLogManager::SetReset(bool blReset)
{
    m_blIsNeedReset = blReset;
}

void CLogManager::ResetLogData(uint16 u2LogLevel)
{
    //重新设置日志等级，加载日志文件
    SetReset(true);

    //这里等待一段时间，等待其他日志全部写入完成，在重载日志模块。
    //这样做少加一个锁
    ACE_Time_Value tvSleep(0, 1000);
    ACE_OS::sleep(tvSleep);

    m_pServerLogger->ReSet(u2LogLevel);
    SetReset(false);
}

uint32 CLogManager::GetLogCount()
{
    if(m_pServerLogger != nullptr)
    {
        return m_pServerLogger->GetLogTypeCount();
    }
    else
    {
        return (uint32)0;
    }
}

uint32 CLogManager::GetCurrLevel()
{
    if(m_pServerLogger != nullptr)
    {
        return m_pServerLogger->GetCurrLevel();
    }
    else
    {
        return (uint32)0;
    }
}

uint16 CLogManager::GetLogID(uint16 u2Index)
{
    if(m_pServerLogger != nullptr)
    {
        return m_pServerLogger->GetLogID(u2Index);
    }
    else
    {
        return (uint16)0;
    }
}

const char* CLogManager::GetLogInfoByServerName(uint16 u2LogID)
{
    if(m_pServerLogger != nullptr)
    {
        return m_pServerLogger->GetLogInfoByServerName(u2LogID);
    }
    else
    {
        return nullptr;
    }
}

const char* CLogManager::GetLogInfoByLogName(uint16 u2LogID)
{
    if(m_pServerLogger != nullptr)
    {
        return m_pServerLogger->GetLogInfoByLogName(u2LogID);
    }
    else
    {
        return nullptr;
    }
}

int CLogManager::GetLogInfoByLogDisplay(uint16 u2LogID)
{
    if(m_pServerLogger != nullptr)
    {
        return m_pServerLogger->GetLogInfoByLogDisplay(u2LogID);
    }
    else
    {
        return 0;
    }
}

uint16 CLogManager::GetLogInfoByLogLevel(uint16 u2LogID)
{
    if(m_pServerLogger != nullptr)
    {
        return m_pServerLogger->GetLogInfoByLogLevel(u2LogID);
    }
    else
    {
        return 0;
    }
}

int CLogManager::WriteLog_i(uint16 u2LogType, string strLog)
{
    //从日志块池里面找到一块空余的日志块
    auto msg = std::make_shared<_LogBlockInfo>();

    msg->m_u4Length = (uint32)strLog.length();
    msg->m_strBlock = strLog;
    msg->m_u2LogID  = u2LogType;

    PutLog(msg);

    return 0;
}

