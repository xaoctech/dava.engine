#ifndef RUNGUARD_H
#define RUNGUARD_H


namespace UTILS
{

    class RunGuard
    {

    public:
        RunGuard( const QString& key );
        ~RunGuard();

        bool isAnotherRunning();
		bool tryToRun();
        void release();

    private:
        const QString m_key;
        const QString m_memLockKey;
        const QString m_sharedmemKey;

        QSharedMemory m_sharedMem;
        QSystemSemaphore m_memLock;

		Q_DISABLE_COPY( RunGuard )
    };

}


#endif // RUNGUARD_H
