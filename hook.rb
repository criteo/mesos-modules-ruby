def slaveRunTaskLabelDecorator name
  puts "about to run task '#{name}'"
end

def slaveRemoveExecutorHook name
  puts "about to remove executor '#{name}'"
  puts "boom!"
  puts 0/0
end

puts "ruby hook loaded"
