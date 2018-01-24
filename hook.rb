def slaveRunTaskLabelDecorator name
  puts "ruby: about to run task '#{name}'"
end

def slaveRemoveExecutorHook name
  puts "ruby: about to remove executor '#{name}'"
end

puts "ruby: hook loaded"
