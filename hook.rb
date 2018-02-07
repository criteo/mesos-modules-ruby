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
