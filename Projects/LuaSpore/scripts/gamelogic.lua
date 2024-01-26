AddOnInitializePropManagerFunction, RemoveOnInitializePropManagerFunction, OnPropManagerInitialized = GenerateCallbackExecuter()

function ExecuteCheatCommand(fn)
    local status, r = pcall(fn)
    if not status then
        print(r)
    end
end