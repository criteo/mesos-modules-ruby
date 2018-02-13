def slaveRunTaskLabelDecorator taskinfo
  puts "[ruby] about to run task '#{taskinfo}'"
  taskinfo["labels"]["foo"] = taskinfo["labels"]["foo"] + "baz"
  taskinfo["labels"]["toto"] = "titi"
  puts "[ruby] decorated labels '#{taskinfo}'"
  return taskinfo
end

def slaveRemoveExecutorHook execinfo
  puts "[ruby] about to remove executor '#{execinfo}'"
end

def slaveExecutorEnvironmentDecorator(executorInfo)
  executorInfo['command']['environment']['foo'] += 'baz'
  executorInfo['command']['environment']['toto'] = 'titi'
  executorInfo['command']['environment'].delete('deleted')
  return executorInfo
end

def isolator_prepare( params )
	pid = Process.pid
	File.open('/tmp/ruby_isolator_stdout.txt','a') do |f| 
    f.puts "<PREP>"
		f.puts "MyPID=#{pid}"
    f.puts "Params (#{params.count}):"
    f.puts "-----------------"
    f.puts params.to_s
    f.puts "</PREP>"
	end
  return {"pre_exec_commands" => [{"value" => "touch /tmp/rb_isolator"}]}
end

def isolator_cleanup( params )
	pid = Process.pid
	File.open('/tmp/ruby_isolator_stdout.txt','a') do |f| 
    f.puts "<CLEAR>"
		f.puts "MyPID=#{pid}"
    f.puts "Params (#{params.count}):"
    f.puts "-----------------"
    f.puts params.to_s
    f.puts "</CLEAR>"
	end	
end
