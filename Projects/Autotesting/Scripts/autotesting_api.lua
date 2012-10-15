print("autotesting_api start")

function Yield()
    print("Yield")
    coroutine.yield()
end

function Wait(time)
    print("Wait "..time)
    local waitTime = 0.0

    while waitTime < time do
        waitTime = waitTime + autotestingSystem:GetTimeElapsed()
        print("Waiting "..waitTime)
        coroutine.yield()
    end
    
    print("Wait done")
end

function WaitControl(time, name)
    print("WaitControl "..time)
    local waitTime = 0.0

    while waitTime < time do
        waitTime = waitTime + autotestingSystem:GetTimeElapsed()
        print("Searching "..waitTime)
        if autotestingSystem:FindControl(name) then
            print("WaitControl found")
            return true
        else
            coroutine.yield()
        end
    end
    
    print("WaitControl done")
    return false
end

function ResumeTest()
    print("ResumeTest")
    if coroutine.status(co) == "suspended" then
        coroutine.resume(co)
        print("ResumeTest done")
    else
        print("ResumeTest failed. status:", coroutine.status(co))
        StopTest()
    end
end

function CreateTest(test)
    print("CreateTest")
    co = coroutine.create(test) -- create a coroutine with foo as the entry
    print(type(co))                 -- display the type of object "co"
    
    print(AutotestingSystem)
    autotestingSystem = AutotestingSystem.Singleton_Autotesting_Instance()
    print("AutotestingSystem.Singleton_Autotesting_Instance")
    
    print(autotestingSystem:GetTimeElapsed())
    
    print("CreateTest done")
end

function StartTest(test)
    print("StartTest")
    CreateTest(test)
    ResumeTest()
end

function StopTest()
    print("StopTest")
    autotestingSystem:StopTest()
end

print("autotesting_api finish")