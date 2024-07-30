AddOnInitializePropManagerFunction, RemoveOnInitializePropManagerFunction, OnPropManagerInitialized = GenerateOrderedCallbackExecuter()

function ExecuteCheatCommand(fn)
    local status, r = pcall(fn)
    if not status then
        return r
    end
end