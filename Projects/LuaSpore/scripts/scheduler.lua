local Task = Class(function(self, fn, delay, ...)
	self.fn = fn
	self.args = {...}
	self.delay = delay
end)

function Task:Execute()
	self.fn(table.unpack(self.args))
end

function Task:Update(dt)
	self.delay = self.delay - dt
	if self.delay <= 0 then
		self:Execute()
	end
	return self.delay <= 0
end

local Periodic = Class(Task, function(self, fn, delay, period, limit, ...)
	Task._ctor(fn, delay, ...)
	self.period = period
	self.limit = limit
end)

function Periodic:Execute()
	Task.Execute(self)
	if self.limit then
		self.limit = self.limit - 1
		if self.limit > 0 then
			self.delay = self.period
		end
	else
		self.delay = self.period
	end
end


local Scheduler = Class(function(self)
	self.tasks = {}
end)

function Scheduler:Update(dt)
	for task in pairs(self.tasks) do
		if not task:Update(dt) then
			self.tasks[task] = nil
		end
	end
end

function Scheduler:CancelTask(task)
	self.tasks[task] = nil
end

function Scheduler:ExecuteInTime(delay, fn, ...)
	local task = Task(fn, delay, ...)
	self.tasks[task] = true
	return task
end

function Scheduler:ExecutePeriodic(period, fn, initialdelay, limit, ...)
	local task = Periodic(fn, initialdelay or period, period, limit, ...)
	self.tasks[task] = true
	return task
end

_G.Scheduler = Scheduler()