require './parser'


GLOBALS = 1
INT = 2
CHAR = 3
FLOAT = 4
ARRAY = 5
BLOCK = 6


BC_VER = [0, 5, 0]
# Last changing: 2024-02-18
# TODO: Add instructions: getfa addff subff mulff; Remove: ; Rename:
class PBC; end
("int char float arr push pushc pusha copy deff setf setfi setfc setff setfa getf getfi getfc getff addfi subfi mulfi defm call ccall ctx gtx block ret rep dupe swap over rot")
  .split.each { |n| PBC.class_eval "#{n.to_s.upcase} = #{n.inspect}" }

$blockc = 16
$block_stack = []


class Token
  def bc
    case @kind
    when Kind::INT then "int #{@value} push #{INT} copy dup dup push #{INT} swap deff @ setfi @\n"
    when Kind::CHAR then "char #{@value.ord} push #{CHAR} copy dup dup push #{CHAR} swap deff @ setfc @\n"
    when Kind::FLOAT then "float #{@value} push #{FLOAT} copy dup dup push #{FLOAT} swap deff @ setff @\n"
    when Kind::STRING then
      code = "gtx gfld Str copy "
      value.each { |c| code += "char #{@value.ord} push #{CHAR} copy dup dup push #{CHAR} swap deff @ setfi @ swap call >> " }
      code + "\n"
    when Kind::DEF_METHOD then "defm #{@value}\n"
    when Kind::CALL_METHOD then "call #{@value}\n"
    when Kind::DEF_CMETHOD then "ctx defm #{@value}\n"
    when Kind::CALL_CMETHOD then "ccall #{@value}\n"
    when Kind::DEF_FIELD then "deff #{@value}\n"
    when Kind::SET_FIELD then "setf #{@value}\n"
    when Kind::GET_FIELD then "gfld #{@value}\n"
    when Kind::DEF_VARIABLE then "gtx deff #{@value}\n"
    when Kind::SET_VARIABLE then "gtx setf #{@value}\n"
    when Kind::GET_VARIABLE then "gtx gfld #{@value}\n"
    when Kind::MACRO then raise "Unexpected macro"
    when Kind::DEF_MACRO then raise "Unexpected macro definition"
    when Kind::BLOCK_OPEN then
      $block_stack.push $blockc
      $blockc += 1
      "block #{$block_stack.last}\n"
    when Kind::BLOCK_CLOSE then "ret block #{$block_stack.pop} push #{BLOCK} copy dup dup push #{BLOCK} swap deff @ push 0 swap setf\n"
    when Kind::ARRAY_OPEN then "pusha\n"
    when Kind::ARRAY_CLOSE then "arr push #{ARRAY} copy dup dup deff @ setfa @\n"
    else raise "Unable to parse token of kind :#{@kind} (value=#{@value})"
    end
  end # bc
end # Token


def compile_macros tokens
  # TODO
  macros = {  }
  compiled = []
  tokens.each do |t|
    if t.kind == Kind::MACRO then t
    elsif t.kind == Kind::DMACRO then t
    else compiled.push t
    end
  end
  compiled
end


def translate_bc(tokens) tokens.map { |t| t.bc }.join + "$" end

