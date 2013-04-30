TIMEOUT = 30.0 -- Big time out for waiting
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
	Yield()
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
			Yield()
			autotestingSystem:WriteState(name, "wait_execution")
			Yield()
			Log("Device "..name.." ready, command was sent")
			return
		elseif state == "error" then 
			OnError("Failed to send job to "..name.." cause error on device: "..command)
		end
		Wait(1)
	end
	OnError("Failed to send job to "..name.." cause timeout: "..command)
end

function WaitJob(name)
	Log("Wait for job on slave "..name)
	local state
	
	for i=1,MULTIPLAYER_TIMEOUT do
		state = autotestingSystem:ReadState(name)
		Yield()
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

function SendJobAndWait(name, command)
	SendJob(name, command)
	WaitJob(name)
end

function noneStep()
	return true
end

-- Work with UI controls
-- !!!!! Realize visibility center of element
function IsVisible(element, background)
	local control = autotestingSystem:FindControl(element)
	if control then
		if background then
			local back = autotestingSystem:FindControl(background)
			assert(back, background.." background not found")
			local geomData = control:GetGeometricData()
            local rect = geomData:GetUnrotatedRect()
			local geomData = back:GetGeometricData()
			local backRect = geomData:GetUnrotatedRect()
            --Log("Control "..tostring(rect.x)..","..tostring(rect.y).." ["..tostring(rect.dx)..", "..tostring(rect.dy).."]")
            --Log("Background "..tostring(backRect.x)..","..tostring(backRect.y).." ["..tostring(backRect.dx)..", "..tostring(backRect.dy).."]")
			
			if (rect.x >= backRect.x) and (rect.x + rect.dx <= backRect.x + backRect.dx) and (rect.y >= backRect.y) and (rect.y + rect.dy <= backRect.y + backRect.dy) then
				return true
			else
				return false
			end
		else
			return true
		end
	else
		return false
	end
end

function IsOnScreen(control)
	local screen = autotestingSystem:GetScreen()
	local geomData = control:GetGeometricData()
    local rect = geomData:GetUnrotatedRect()
	local geomData = screen:GetGeometricData()
	local backRect = geomData:GetUnrotatedRect()
    Log("Control "..tostring(rect.x)..","..tostring(rect.y).." ["..tostring(rect.dx)..", "..tostring(rect.dy).."]")
    Log("Background "..tostring(backRect.x)..","..tostring(backRect.y).." ["..tostring(backRect.dx)..", "..tostring(backRect.dy).."]")
			
	if (rect.x >= backRect.x) and (rect.x + rect.dx <= backRect.x + backRect.dx) and (rect.y >= backRect.y) and (rect.y + rect.dy <= backRect.y + backRect.dy) then
		return true
	else
		return false
	end
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
            Wait(1)
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
    local position = Vector.Vector2(x, y)
    autotestingSystem:TouchMove(position, touchId)
    Wait(waitTime)
end

function TouchUp(touchId)
	local touchId = touchId or 1
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
        local screen = autotestingSystem:GetScreen()
        if control and IsOnScreen(control) then     
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
			Wait(DELAY)
            Yield()
        end
    end
    
    Log("ClickControl not found "..name)
    return false
end

-- Work with Text field and labels 

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

-- Work with List
function SelectHorizontal(list, item)
	Log("Select "..tostring(item).." item in horizontal list "..list)
	local cell = list.."/".. tostring(item)
	assert(WaitControl(list), "Couldn't select "..cell)
	
		local last_visible = 0
	local previous_last = 0
	local index = 0
	
	-- find first visible element
	for i = 0, 100 do --to avoid hanging up in empty list
		if IsVisible(list.."/"..tostring(i)) then
			previous_last = i
			last_visible = i
			Log( "previous_last = "..tostring(previous_last)..",last_visible = "..tostring(last_visible) )
			break
		end
	end
    
	-- find last wisible
	index = previous_last + 1
	while true do
		if not IsVisible(list.."/"..tostring(index)) then
			last_visible = index - 1
			Log( "last_visible = "..tostring(last_visible) )
			break
		end
		index = index + 1
	end
	
	repeat
		if IsVisible(cell, list) then
			break
		else
			previous_last = last_visible
			ScrollLeft(list)
			index = last_visible + 1
			while true do
				if not IsVisible(list.."/"..tostring(index)) then
					last_visible = index - 1
					Log( "previous_last = "..tostring(previous_last) )
					break
				end
				index = index + 1
			end
		end
	until previous_last == last_visible
    
	if IsVisible(cell, list) then
		ClickControl(cell)
		return true
	else
		Log("List "..list.." not found")
		return false
	end
end

function SelectVertical(list, item)
	Log("Select "..tostring(item).." item in vertical list "..list)
	local cell = list.."/".. tostring(item)
	assert(WaitControl(list), "Couldn't select "..cell)
	
	local last_visible = 0
	local previous_last = 0
	local index = 0
	
	-- find first visible element
	for i = 0, 100 do --to avoid hanging up in empty list
		if IsVisible(list.."/"..tostring(i)) then
			previous_last = i
			last_visible = i
			Log( "previous_last = "..tostring(previous_last)..",last_visible = "..tostring(last_visible) )
			break
		end
	end
    
	-- find last wisible
	index = previous_last + 1
	while true do
		if not IsVisible(list.."/"..tostring(index)) then
			last_visible = index - 1
			Log( "last_visible = "..tostring(last_visible) )
			break
		end
		index = index + 1
	end
	
	repeat
		if IsVisible(cell, list) then
			break
		else
			previous_last = last_visible
			ScrollDown(list)
			index = last_visible + 1
			while true do
				if not IsVisible(list.."/"..tostring(index)) then
					last_visible = index - 1
					Log( "previous_last = "..tostring(previous_last) )
					break
				end
				index = index + 1
			end
		end
	until previous_last == last_visible
	
	if IsVisible(cell, list) then
		ClickControl(cell)
		return true
	else
		Log("List "..list.." not found")
		return false
	end
end

function ScrollDown(list)
	local control = autotestingSystem:FindControl(list)
    if control then	
        local position = Vector.Vector2()
            
        local geomData = control:GetGeometricData()
        local rect = geomData:GetUnrotatedRect()
       
        position.x = rect.x + rect.dx/2
        position.y = rect.y + rect.dy/2
		
		TouchDownPosition(position)
		Wait(0.5)
        position.y = position.y - rect.dy/3
		TouchMovePosition(position)
		TouchUp()
		Wait(1)
	else
		OnError("Couldnt find list "..list)
	end
end

function ScrollLeft(list)
	local control = autotestingSystem:FindControl(list)
    if control then	
        local position = Vector.Vector2()
            
        local geomData = control:GetGeometricData()
        local rect = geomData:GetUnrotatedRect()
       
        position.x = rect.x + rect.dx/2
        position.y = rect.y + rect.dy/2
		
		TouchDownPosition(position)
		Wait(0.5)
        position.x = position.x - rect.dx/3
		TouchMovePosition(position)
		TouchUp()
		Wait(1)
	else
		OnError("Couldnt find list "..list)
	end
end

-- !!!!!!!Realize from C side
function GetCellsCount(element)
	-- cause index begin from '0'
	return 1 - 1
end