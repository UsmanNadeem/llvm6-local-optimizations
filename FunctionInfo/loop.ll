; ModuleID = 'loop.c'
source_filename = "loop.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@g = common local_unnamed_addr global i32 0, align 4

; Function Attrs: norecurse nounwind uwtable
define i32 @change_g(i32) local_unnamed_addr #0 {
  %2 = load i32, i32* @g, align 4, !tbaa !2
  %3 = add nsw i32 %2, %0
  store i32 %3, i32* @g, align 4, !tbaa !2
  ret i32 %3
}

; Function Attrs: norecurse nounwind uwtable
define i32 @sample(i32, i32) local_unnamed_addr #0 {
  %3 = icmp sgt i32 %0, 0
  %4 = load i32, i32* @g, align 4, !tbaa !2
  br i1 %3, label %5, label %8

; <label>:5:                                      ; preds = %2
  %6 = mul i32 %1, %0
  %7 = add i32 %4, %6
  store i32 %7, i32* @g, align 4, !tbaa !2
  br label %8

; <label>:8:                                      ; preds = %5, %2
  %9 = phi i32 [ %7, %5 ], [ %4, %2 ]
  ret i32 %9
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0-1ubuntu2 (tags/RELEASE_600/final)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
