#!/usr/bin/env ruby

if ARGV.size != 2
  puts "Usage: #{File.basename($0)} INFILE OUTFILE"
  exit 1
end

cpp = false

File.open(ARGV[1], "w") do |out|
  File.open(ARGV[0]).each do |line|
    if cpp
      if /^~~~~$/ =~ line
        cpp = false
      else
        out.write(line)
      end
    else
      if /^~~~~cpp$/ =~ line
        cpp = true
      end
    end
  end
end
