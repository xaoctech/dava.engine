-- module setup
local modname = ...
local M = {}
_G[modname] = M
package.loaded[modname] = M

setmetatable(M, {__index = _G})

-- Import Section:
-- declare everything module needs from outside
local cprint = print

-- no  more external access after this point
--setfenv(1, M)


-- File setup
-- !!! Remove after testing
local log = io.open("test_log.txt", "w")
log:close()



-- Functions
function print(txt)
	local log = io.open("test_log.txt", "a")
	cprint(txt)
	
	local function foo()
		log:write(txt)
	end
	
	local status, err = pcall(foo)
	if not status then
		log:write("Couldn't write to file:\n" .. err)
	end
	
	log:write("\n")
	log:flush()
	log:close()
end

