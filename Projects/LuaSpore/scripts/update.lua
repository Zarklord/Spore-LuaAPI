local ExecuteUpdateFunctions
AddUpdateFunction, RemoveUpdateFunction, ExecuteUpdateFunctions = GenerateCallbackExecuter()

function Update(dt)
	Scheduler:Update(dt)

	ExecuteUpdateFunctions()
end