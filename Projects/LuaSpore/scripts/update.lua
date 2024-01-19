local update_fns = {}

function AddUpdateFunction(fn)
	update_fns[fn] = true
end

function RemoveUpdateFunction(fn)
	update_fns[fn] = nil
end

function Update(dt)
	Scheduler:Update(dt)

	for update_fn in pairs(update_fns) do
		update_fn(dt)
	end
end