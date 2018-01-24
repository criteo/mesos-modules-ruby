def slaveRunTaskLabelDecorator name
  puts "ruby: about to run task '#{name}'"
end

def slaveRemoveExecutorHook name
  puts "ruby: about to remove executor '#{name}'"
  puts "ruby: boom!"
  puts 0/0
end

puts "ruby: hook loaded"
