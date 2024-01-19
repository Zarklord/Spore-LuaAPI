function string.split(str, sep, plain)
	sep = sep or ":"
	plain = plain ~= false
	local strs = {}
	local current_str_pos = 1
	while true do
		local pattern_start, pattern_end = string.find(str, sep, current_str_pos, plain)
		if pattern_start then
			table.insert(strs, string.sub(str, current_str_pos, pattern_start-1))
			current_str_pos = pattern_end+1
		else
			table.insert(strs, string.sub(str, current_str_pos))
			break
		end
	end
	return strs
end

function string.count(str, char, plain)
	plain = plain ~= false
	local count = 0
	local current_str_pos = 1
	while true do
		local pattern_start, pattern_end = string.find(str, char, current_str_pos, plain)
		if pattern_start then
			count = count + 1
			current_str_pos = pattern_end+1
		else
			break
		end
	end
	return count
end