TIMEOUT = 20.0 -- Big time out for waiting
TIMECLICK = 0.2 -- time for simple action
DELAY = 0.5 -- time for simulation of human reaction

MULTIPLAYER_TIMEOUT = 300 -- Multiplayer timeout

-- API setup
function SetPackagePath(path)
	package.path = package.path .. ";" .. path .. "Actions/?.lua;" .. path .. "Scripts/?.lua;"
	
	--require "logger"
	require "coxpcall"
end


-- High-level test function
----------------------------------------------------------------------------------------------------
-- This function for simple test step without any assertion. Fail while error throwing
-- Parameters:
--		description - step description
--		func - link to step function
--		... - input parameters for step function
function Step(description, func, ...)
    autotestingSystem:OnStepStart(description)
	
	local status, err = copcall(func, ...)

	if not status then OnError(err) end
	Yield()
end

-- This function for test step with assertion. Fail when step is returned NIL or FALSE
-- Parameters:
--		description - step description
--		func - link to step function
--		... - input parameters for step function
function Assert(description, func, ...)
    autotestingSystem:OnStepStart(description)
	
	local status, err = copcall(func, ...)

	if not status then
		-- Some error during test step
		OnError(err)
	elseif not err then
		OnError("Assertion failed, expected result not equal to actual")
	end
	Yield()
end

function Log(message, level)
	level = level or "DEBUG"
		autotestingSystem:Log(level, tostring(message))
end 

--
function Yield()
    coroutine.yield()
end

function ResumeTest()
    --print("ResumeTest")
    if coroutine.status(co) == "suspended" then
        coroutine.resume(co)
        --print("ResumeTest done")
    else
        --print("ResumeTest failed. status:", coroutine.status(co))
        StopTest()
    end
end

function CreateTest(test)
    --print("CreateTest")
    co = coroutine.create(test) -- create a coroutine with foo as the entry
    autotestingSystem = AutotestingSystem.Singleton_Autotesting_Instance()
    
    --print(autotestingSystem:GetTimeElapsed())	
end

function StartTest(name, test)      
    CreateTest(test)
	--print('StartTest')
	autotestingSystem:OnTestStart(name)
    Yield()
end

function OnError(text)
	autotestingSystem:OnError(text)
	Yield()
end

function StopTest()
--    print("StopTest")
    autotestingSystem:OnTestFinished()
end

----------------------------------------------------------------------------------------------------
-- Multiplayer API
----------------------------------------------------------------------------------------------------
-- mark current device as ready to work in DB
function InitializeDevice(name)
	DEVICE = name
	Log("Mark "..name.." device as Ready")
	autotestingSystem:InitializeDevice(DEVICE)
	autotestingSystem:WriteState(DEVICE, "ready")
end

function WaitForDevice(name)
	Log("Wait while "..name.." device become Ready")
	for i=1,MULTIPLAYER_TIMEOUT do
		if autotestingSystem:ReadState(name) == "ready" then
			return
		else
			Wait(1)
		end
	end
	OnError("Device "..name.." is not ready during timeout")
end

function SendJob(name, command)
	Log("Send to slave "..name.." command: "..command)
		
	for i=1,MULTIPLAYER_TIMEOUT do
		local state = autotestingSystem:ReadState(name)
		if state == "ready" then
			autotestingSystem:WriteCommand(name, command)
			autotestingSystem:WriteState(name, "wait_execution")
			Log("Device "..name.." ready, comand was sent")
			return
		elseif state == "error" then 
			OnError("Failed to send job to "..name.." cause error on device: "..command)
		end
		Wait(1)
	end
	OnError("Failed to send job to "..name.." cause timeout: "..command)
end

function WaitJob(name)
	Log("Wait for slave "..name)
	local state
	
	for i=1,MULTIPLAYER_TIMEOUT do
		state = autotestingSystem:ReadState(name)
		if state == "execution_completed" then
			autotestingSystem:WriteState(name, "ready")
			Log("Device "..name.." finish his job")
			return
		elseif state == "error" then
			OnError("Error on "..name.." device")
		else
			Wait(1)
		end
	end
	
	OnError("Wait for job on "..name.." device failed by timeout. Last state "..state)
end


function Wait(waitTime)
    waitTime =  waitTime or DELAY
    
    local elapsedTime = 0.0
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
        Yield()
    end
end

function WaitControl(name, time)
    local waitTime = time or TIMEOUT
    Log("WaitControl name="..name.." waitTime="..waitTime,"DEBUG")
    
    local elapsedTime = 0.0
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
--        print("Searching "..elapsedTime)
        
        if autotestingSystem:FindControl(name) then
            Log("WaitControl found "..name, "DEBUG")
            Yield()
            return true
        else
            Yield()
        end
    end
    
    Log("WaitControl not found "..name, "DEBUG")
    return false
end

function TouchDownPosition(position, touchId)
    local touchId = touchId or 1
    --print("TouchDownPosition position="..position.x..","..position.y.." touchId="..touchId)
    autotestingSystem:TouchDown(position, touchId)
end

function TouchDown(x, y, touchId)
    local touchId = touchId or 1
    --print("TouchDown x="..x.." y="..y.." touchId="..touchId)
    autotestingSystem:TouchDown(position, touchId)
end

function TouchMovePosition(position, touchId, waitTime)
	waitTime =  waitTime or TIMECLICK
    local touchId = touchId or 1
    Log("TouchMovePosition position="..position.x..","..position.y.." touchId="..touchId)
    autotestingSystem:TouchMove(position, touchId)
    Wait(waitTime)
end

function TouchMove(x, y, touchId, waitTime)
	waitTime =  waitTime or TIMECLICK
    local touchId = touchId or 1
    Log("TouchMove x="..x.." y="..y.." touchId="..touchId)
    autotestingSystem:TouchMove(position, touchId)
    Wait(waitTime)
end

function TouchUp(touchId)
    Log("TouchUp "..touchId)
    autotestingSystem:TouchUp(touchId)
end

function ClickPosition(position, touchId, time)
    local waitTime = time or TIMECLICK
    local touchId = touchId or 1
    Log("ClickPosition position="..position.x..","..position.y.." touchId="..touchId.." waitTime="..waitTime)
    
    Wait(waitTime)
    TouchDownPosition(position, touchId)
    Wait(TIMECLICK)
    TouchUp(touchId)
    Wait(waitTime)
end

function Click(x, y, touchId)
    local waitTime = time or TIMECLICK
    local touchId = touchId or 1
    --print("Click x="..x.." y="..y.." touchId="..touchId)
    
    local position = Vector.Vector2(x, y)
    ClickPosition(position, touchId, waitTime)
end

function ClickControl(name, touchId, time)
    local waitTime = time or TIMECLICK
    local touchId = touchId or 1
	
    Log("ClickControl name="..name.." touchId="..touchId.." waitTime="..waitTime)
    
    local elapsedTime = 0.0
    while elapsedTime < TIMEOUT do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
--        print("Searching "..elapsedTime)
        
        local control = autotestingSystem:FindControl(name)
        if control then
--            print("ClickControl found "..name)
--            print(control)
            
            -- local position = control:GetPosition(true)
            local position = Vector.Vector2()
--            print(position)
--            print("position="..position.x..","..position.y)
            
            local geomData = control:GetGeometricData()
--            print(geomData)
            local rect = geomData:GetUnrotatedRect()
--            print(rect)
            
--            print("rect.x="..rect.x)
--            print("rect.y="..rect.y)
--            print("rect.dx="..rect.dx)
--            print("rect.dy="..rect.dy)
            
            position.x = rect.x + rect.dx/2
            position.y = rect.y +rect.dy/2
--            print("position="..position.x..","..position.y)
            
            ClickPosition(position, touchId, waitTime)
            
            return true
        else
            Yield()
        end
    end
    
    Log("ClickControl not found "..name)
    return false
end

function SetText(path, text, time)
	local waitTime = time or DELAY
    Log("SetText path="..path.." text="..text)
    local res = autotestingSystem:SetText(path, text)
    Yield()
    Wait(waitTime)
    return res
end

function CheckText(name, txt)
	Log("Check that text '" .. txt .. "' is present on control " .. name)
	local control = autotestingSystem:FindControl(name)
	
	if control then
		Wait(waitTime)
		return autotestingSystem:CheckText(control, txt)
	else
		error("Control " .. name .. " not found")
	end
end

function CheckMsgText(name, key)
	Log("Check that text with key [" .. key .. "] is present on control " .. name)
	local control = autotestingSystem:FindControl(name)
	
	if control then
		Wait(waitTime)
		return autotestingSystem:CheckMsgText(control, key)
	else
		error("Control " .. name .. " not found")
	end
end
