WORD = /[A-Za-z_~%^&*\-+=|<>\/?\\][A-Za-z0-9_~%^&*\-+=|<>\/?\\]*/
TOKEN = /\"[^"]*?\"|-?\d+(\.\d+)?|'.|\s+|(:!|:@|=@|:\$|=\$|:#|[:!@#$])?#{WORD}|./
# NOTE: This reges depends to the bytecode version, that is defining in translator.rb


class InvalidTokenError < Exception; end


class Kind; end
[
  :int, :char, :float, :string,
  :def_method, :call_method, :def_cmethod, :call_cmethod,
  :def_field, :set_field, :get_field, :def_variable, :set_variable, :get_variable,
  :dmacro, :macro,
  :block_open, :block_close, :array_open, :array_close,
].each { |n| Kind.class_eval "#{n.upcase} = :#{n}" }

class Token
  attr_reader :kind
  attr_reader :value
  def initialize kind, value=nil
    @kind = kind
    @value = value
  end
  def to_s; @value.to_s.inspect end
  def inspect; "[[#{@kind}:#{@value.inspect}]]" end
end # Token


def parse str
  tokens_src = []
  str.scan(TOKEN) { tokens_src.push $~[0] }
  tokens = tokens_src.map do |s|
    case s
    when /^-?\d+$/ then Token.new Kind::INT, s.to_i
    when /^-?\d+\.\d+$/ then Token.new Kind::FLOAT, s.to_f
    when /^'.$/ then Token.new Kind::CHAR, s[1]
    when /^"[^"]*"$/ then Token.new Kind::STRING, eval(s)
    when /^:#{WORD}$/ then Token.new Kind::DEF_METHOD, s[1..]
    when /^#{WORD}$/ then Token.new Kind::CALL_METHOD, s
    when /^:!#{WORD}$/ then Token.new Kind::DEF_CMETHOD, s[2..]
    when /^!#{WORD}$/ then Token.new Kind::CALL_CMETHOD, s[1..]
    when /^:@#{WORD}$/ then Token.new Kind::DEF_FIELD, s[2..]
    when /^=@#{WORD}$/ then Token.new Kind::SET_FIELD, s[2..]
    when /^@#{WORD}$/ then Token.new Kind::GET_FIELD, s[1..]
    when /^:\$#{WORD}$/ then Token.new Kind::DEF_VARIABLE, s[2..]
    when /^=\$#{WORD}$/ then Token.new Kind::SET_VARIABLE, s[2..]
    when /^\$#{WORD}$/ then Token.new Kind::GET_VARIABLE, s[1..]
    when /^:##{WORD}$/ then Token.new Kind::DEF_MACRO, s[2..]
    when /^##{WORD}$/ then Token.new Kind::MACRO, s[1..]
    when /^\s+$/ then nil # skip this token
    else raise InvalidTokenError.new s
    end
  end.select(&:itself)
  # TODO: Compile macros
  tokens.map { |t| if t.kind == Kind::STRING then compile_string t.value else t end }.flatten
end # parse


def compile_string s
  result = [Token.new(Kind::GET_VARIABLE, 'String'), Token.new(Kind::CALL_METHOD, 'copy')]
  s.each_char.map { |c| result.push Token.new(Kind::CALL_METHOD, '++'), Token.new(Kind::CHAR, c) }
  result
end

