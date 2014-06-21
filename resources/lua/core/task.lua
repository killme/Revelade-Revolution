
local function createTask(callback, cancelCallback)
    local references = 0
    
    local function addTask()
        references = references + 1
        
        --[[
        print (("addTask(%i): %s "):format(references, require "debug".traceback()))
        ]]
        
        return true
    end
    
    local function finishTask()
        references = references - 1
        
        --[[
        print (("finishTask(%i): %s "):format(references, require "debug".traceback()))
        ]]
        
        if references == 0 then
            callback()
        end
    end
    
    local function cancel(...)
        references = -1
        cancelCallback(...) 
    end
        
    
    return {
        addTask = addTask,
        finishTask = finishTask,
        cancel = cancel
    }
end

return {
    createTask = createTask
}