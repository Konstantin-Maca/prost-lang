require './parser'
require './translator'

HELP_MESSAGE = "prostc file.prs"

def output_filename name; name.match(/(.*)\.prs|(.*)/)[1..2].find { |e| !e.nil? } + '.pbc' end

if ARGV.size == 1
  File.write(
    output_filename(ARGV[0]),
    translate_bc(parse File.read ARGV[0])
  )
else push HELP_MESSAGE
end

