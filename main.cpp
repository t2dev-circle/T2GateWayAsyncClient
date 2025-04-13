#include <iostream>
#include "servant/Communicator.h"
#include "T2TarsObj.h"
#include "util/tc_timeprovider.h"
#include <atomic>

using namespace std;
using namespace T2App;
using namespace tars;

static bool packLogin(T2TarsRequest &req)
{
    int funcNo = 331100;
    map<string, string> data;

    data["func_no"] = std::to_string(funcNo);
    data["op_branch_no"] = "1000";
    data["op_entrust_way"] = "7";
    data["op_station"] = "127.0.0.1";
    data["input_content"] = "1";
    data["account_content"] = "123456";
    data["content_type"] = "0";
    data["password"] = "111111";
    data["asset_prop"] = "0";

    req.funcNo = funcNo;
    req.seqNo = 1;
    req.data.swap(data);

    return true;
}

static bool packQueryStock(T2TarsRequest &req)
{
    int funcNo = 330300;
    map<string, string> data;

    data["func_no"] = std::to_string(funcNo);
    data["op_branch_no"] = "1000";
    data["op_entrust_way"] = "7";
    data["op_station"] = "127.0.0.1";
    data["stock_code"] = "000001";

    req.funcNo = funcNo;
    req.seqNo = 1;
    req.data.swap(data);

    return true;
}

static bool packQueryAsset(T2TarsRequest &req)
{
    int funcNo = 332255;
    map<string, string> data;

    data["func_no"] = std::to_string(funcNo);
    data["op_branch_no"] = "1000";
    data["op_entrust_way"] = "7";
    data["op_station"] = "127.0.0.1";
    data["client_id"] = "123456";
    data["fund_account"] = "123456";
    data["password"] = "111111";

    req.funcNo = funcNo;
    req.seqNo = 1;
    req.data.swap(data);

    return true;
}

static bool packQueryPosition(T2TarsRequest &req)
{
    int funcNo = 333104;
    map<string, string> data;

    data["func_no"] = std::to_string(funcNo);
    data["op_branch_no"] = "1000";
    data["op_entrust_way"] = "7";
    data["op_station"] = "127.0.0.1";
    data["client_id"] = "123456";
    data["fund_account"] = "123456";
    data["password"] = "111111";

    req.funcNo = funcNo;
    req.seqNo = 1;
    req.data.swap(data);

    return true;
}

class Analyzer : public TC_HandleBase
{
public:
    Analyzer(int reqCount)
    {
        m_reqCount = reqCount;
        m_okCount = 0;
        m_failCount = 0;
        m_beginTime = TC_TimeProvider::getInstance()->getNowMs();
    }
    ~Analyzer()
    {
    }
public:
    void addOK()
    {
        m_okCount++;

        if (m_okCount == m_reqCount)
        {
            double data = getQPS();
            cout << "qps=" << data << endl;
        }
    }
    void addFail()
    {
        m_failCount++;
    }

    int getOK()
    {
        return m_okCount;
    }

    int getFail()
    {
        return m_failCount;
    }

    time_t getInterval()
    {
        return TC_TimeProvider::getInstance()->getNowMs() - m_beginTime;
    }

    double getQPS()
    {
        if ((m_failCount > 0) || (m_okCount != m_reqCount))
        {
            return 1;
        }

        time_t interval = getInterval();
        if (interval <= 0)
        {
            return 1;
        }

        cout << "req=" << m_reqCount << "|interval=" << interval << "ms" << endl;

        return m_reqCount / (interval / 1000.0);
    }
private:
    time_t m_beginTime;
    int m_reqCount;
    std::atomic<int> m_okCount;
    std::atomic<int> m_failCount;
};

typedef tars::TC_AutoPtr<Analyzer> AnalyzerPtr;

class AsyncCallback : public T2TarsObjPrxCallback
{
public:
    AsyncCallback(AnalyzerPtr ptr)
    {
        m_ptr = ptr;
    }
    ~AsyncCallback()
    {
    }
public:
    virtual void callback_invoke(tars::Int32 ret,  const T2App::T2TarsResponse& rsp)
    {
        //cout << ret << "|" << rsp.writeToJsonString() << endl;

        m_ptr->addOK();
    }

    virtual void callback_invoke_exception(tars::Int32 ret)
    {
        m_ptr->addFail();
    }
private:
    AnalyzerPtr m_ptr;
};

typedef tars::TC_AutoPtr<AsyncCallback> AsyncCallbackPtr;



int main(int argc,char ** argv)
{
    Communicator comm;
    T2TarsObjPrx prx;

    int reqCount = 100000;
    AnalyzerPtr analyzerPtr = new Analyzer(reqCount);

    try
    {
        comm.stringToProxy("T2App.T2GateWayServer.T2TarsObj@tcp -h 172.19.16.202 -p 18899 -t 60000", prx);
        cout << "prx: " << prx << endl;
        
        for (int i = 0; i < reqCount; i++)
        {
            T2TarsRequest req;

            packQueryStock(req);

            AsyncCallbackPtr ptr = new AsyncCallback(analyzerPtr);
            prx->async_invoke(ptr, req);
        }
    }
    catch(exception &ex)
    {
        cerr << "exception:" << ex.what() << endl;
    }
    catch(...)
    {
        cerr << "unknown exception" << endl;
    }

    getchar();

    return 0;
}