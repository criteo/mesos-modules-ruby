def slaveRunTaskLabelDecorator taskinfo
  puts "[ruby] about to run task '#{taskinfo}'"
  taskinfo["labels"]["foo"] = taskinfo["labels"]["foo"] + "baz"
  taskinfo["labels"]["toto"] = "titi"
  puts "[ruby] decorated labels '#{taskinfo}'"
  return taskinfo
end
