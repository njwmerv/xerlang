; ModuleID = 'xerlang_compiler'
source_filename = "xerlang_compiler"

@a = common local_unnamed_addr global i8 0
@x = common local_unnamed_addr global i32 0
@AB = common local_unnamed_addr global i32 -1
@read_fmt = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@print_fmt.2 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn
define i1 @guhh() local_unnamed_addr #0 {
entry:
  ret i1 false
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn
define i1 @is_capitalized(i8 %c) local_unnamed_addr #0 {
entry:
  %0 = add i8 %c, -65
  %1 = icmp ult i8 %0, 3
  %sc_zext9 = zext i1 %1 to i32
  ret i32 %sc_zext9
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn
define void @incr() local_unnamed_addr #1 {
entry:
  %x_load_tmp = load i32, ptr @x, align 4
  %add_tmp = add i32 %x_load_tmp, 1
  store i32 %add_tmp, ptr @x, align 4
  ret void
}

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind willreturn
define void @decr(ptr nocapture %x, i32 %y) local_unnamed_addr #2 {
entry:
  %deref_tmp = load i32, ptr %x, align 4
  %sub_tmp = sub i32 %deref_tmp, %y
  store i32 %sub_tmp, ptr %x, align 4
  ret void
}

; Function Attrs: nounwind
define i32 @main() local_unnamed_addr #3 {
entry:
  %read_tmp = alloca i32, align 4
  %scanf_call = call i32 (ptr, ...) @scanf(ptr nonnull @read_fmt, ptr nonnull %read_tmp)
  %read_val = load i32, ptr %read_tmp, align 4
  %char_trunc_tmp = trunc i32 %read_val to i8
  %eq_tmp = icmp eq i8 %char_trunc_tmp, 97
  br i1 %eq_tmp, label %then, label %elif_fallthrough

then:                                             ; preds = %entry
  %putchar42 = call i32 @putchar(i32 97)
  br label %if_merge23

elif_fallthrough:                                 ; preds = %entry
  %0 = add i8 %char_trunc_tmp, -65
  %1 = icmp ult i8 %0, 3
  %sc_zext9.i = zext i1 %1 to i32
  br i32 %sc_zext9.i, label %then3, label %for_body.preheader

for_body.preheader:                               ; preds = %elif_fallthrough
  %printf_call12 = call i32 (ptr, ...) @printf(ptr nonnull @print_fmt.2, i32 undef)
  %printf_call12.1 = call i32 (ptr, ...) @printf(ptr nonnull @print_fmt.2, i32 undef)
  %printf_call12.2 = call i32 (ptr, ...) @printf(ptr nonnull @print_fmt.2, i32 undef)
  %printf_call12.3 = call i32 (ptr, ...) @printf(ptr nonnull @print_fmt.2, i32 undef)
  %printf_call12.4 = call i32 (ptr, ...) @printf(ptr nonnull @print_fmt.2, i32 undef)
  %printf_call12.5 = call i32 (ptr, ...) @printf(ptr nonnull @print_fmt.2, i32 undef)
  %printf_call12.6 = call i32 (ptr, ...) @printf(ptr nonnull @print_fmt.2, i32 undef)
  %printf_call12.7 = call i32 (ptr, ...) @printf(ptr nonnull @print_fmt.2, i32 undef)
  %printf_call12.8 = call i32 (ptr, ...) @printf(ptr nonnull @print_fmt.2, i32 undef)
  %printf_call12.9 = call i32 (ptr, ...) @printf(ptr nonnull @print_fmt.2, i32 undef)
  %neq_tmp.not.old = icmp eq i8 %char_trunc_tmp, 5
  br i1 %neq_tmp.not.old, label %if_merge23, label %while_body.preheader

then3:                                            ; preds = %elif_fallthrough
  %x_load_tmp = load i32, ptr @x, align 4
  %printf_call4 = call i32 (ptr, ...) @printf(ptr nonnull @print_fmt.2, i32 %x_load_tmp)
  br label %if_merge23

while_body.preheader:                             ; preds = %for_body.preheader
  %chari40 = sext i8 %char_trunc_tmp to i32
  %if_cond.not.not = icmp eq i8 %char_trunc_tmp, 0
  br label %while_body

while_body:                                       ; preds = %while_body.preheader, %while_body
  %putchar41 = call i32 @putchar(i32 %chari40)
  br i1 %if_cond.not.not, label %while_body, label %if_merge23

if_merge23:                                       ; preds = %while_body, %for_body.preheader, %then3, %then
  %putchar = call i32 @putchar(i32 97)
  ret i32 0
}

; Function Attrs: nofree nounwind
declare noundef i32 @scanf(ptr nocapture noundef readonly, ...) local_unnamed_addr #4

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #4

; Function Attrs: nofree nounwind
declare noundef i32 @putchar(i32 noundef) local_unnamed_addr #4

attributes #0 = { mustprogress nofree norecurse nosync nounwind readnone willreturn }
attributes #1 = { mustprogress nofree norecurse nosync nounwind willreturn }
attributes #2 = { argmemonly mustprogress nofree norecurse nosync nounwind willreturn }
attributes #3 = { nounwind }
attributes #4 = { nofree nounwind }
