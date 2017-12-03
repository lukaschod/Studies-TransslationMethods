call Main
halt

func Power2 1 0
  ldarg.0
  ldarg.0
  mul
  ret

func Main 0 3
  ldloca.0
  ldc.4 10
  sta.4
  ldloca.1
  ldc.4 25
  sta.4
  ldloca.2
  ldloc.0
  ldloc.1
  add
  ldc.4 500
  mul
  sta.4
  ldc.4 50
  ldc.4 80
  ldloc.2
  div
  mul
  ret
  ldloc.2
  ldc.4 2
  ceq
  brfalse LABEL_0
  ldloca.2
  ldc.4 10
  call Power2
  sta.4
  .label LABEL_0
  ldc.4 10
  call Power2
  retskip
  ldloc.2
  ldc.4 10
  ceq
  retskip

