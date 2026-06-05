; ModuleID = 'xerlang_compiler'
source_filename = "xerlang_compiler"

declare ptr @malloc(i64)

declare void @free(ptr)

define i1 @guhh() {
entry:
  ret i1 false
}

define i1 @is_capitalized(i8 %c) {
entry:
  %c1 = alloca i8, align 1
  store i8 %c, ptr %c1, align 1
  ret i1 false
}

define void @incr() {
entry:
  ret void
}

define void @decr(ptr %x, i32 %y) {
entry:
  %x1 = alloca ptr, align 8
  store ptr %x, ptr %x1, align 8
  %y2 = alloca i32, align 4
  store i32 %y, ptr %y2, align 4
  ret void
}

define i32 @main() {
entry:
  ret i32 0
}
