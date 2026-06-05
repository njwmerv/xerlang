; ModuleID = 'xerlang_compiler'
source_filename = "xerlang_compiler"

@fmt_char = private unnamed_addr constant [4 x i8] c"%c\0A\00", align 1

; Function Attrs: nofree nounwind
define i32 @main() local_unnamed_addr #0 {
entry:
  %printf_call_char = tail call i32 (ptr, ...) @printf(ptr nonnull @fmt_char, i8 72)
  %printf_call_char.1 = tail call i32 (ptr, ...) @printf(ptr nonnull @fmt_char, i8 101)
  %printf_call_char.2 = tail call i32 (ptr, ...) @printf(ptr nonnull @fmt_char, i8 108)
  %printf_call_char.3 = tail call i32 (ptr, ...) @printf(ptr nonnull @fmt_char, i8 108)
  %printf_call_char.4 = tail call i32 (ptr, ...) @printf(ptr nonnull @fmt_char, i8 111)
  %printf_call_char.5 = tail call i32 (ptr, ...) @printf(ptr nonnull @fmt_char, i8 44)
  %printf_call_char.6 = tail call i32 (ptr, ...) @printf(ptr nonnull @fmt_char, i8 32)
  %printf_call_char.7 = tail call i32 (ptr, ...) @printf(ptr nonnull @fmt_char, i8 87)
  %printf_call_char.8 = tail call i32 (ptr, ...) @printf(ptr nonnull @fmt_char, i8 111)
  %printf_call_char.9 = tail call i32 (ptr, ...) @printf(ptr nonnull @fmt_char, i8 114)
  %printf_call_char.10 = tail call i32 (ptr, ...) @printf(ptr nonnull @fmt_char, i8 108)
  %printf_call_char.11 = tail call i32 (ptr, ...) @printf(ptr nonnull @fmt_char, i8 100)
  %printf_call_char.12 = tail call i32 (ptr, ...) @printf(ptr nonnull @fmt_char, i8 33)
  %printf_call_char.13 = tail call i32 (ptr, ...) @printf(ptr nonnull @fmt_char, i8 110)
  ret i32 0
}

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #0

attributes #0 = { nofree nounwind }
