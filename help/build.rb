#!/usr/bin/ruby

require 'yaml'
require 'erb'

root = File.expand_path(File.dirname(__FILE__))
directives = YAML.load_file("#{root}/directive.yml")
syntax = ERB.new(File.read("#{root}/syntax.erb"), nil, '%')
File.write("#{root}/syntax.html", syntax.result(binding))
