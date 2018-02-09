

require 'yaml'

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
end

def isolator_cleanup( params )
	pid = Process.pid
	File.open('/tmp/ruby_isolator_stdout.txt','a') do |f| 
		f.puts "[CLEAN] MyPID=#{pid}"
	end	
end

