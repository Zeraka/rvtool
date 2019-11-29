该样例，产生 event1, event2, event3 ,event4这样的日志。并且拥有顺序。

验证的性质是, 当event3出现后event4立即出现，并且event1 和event2不会再出现。

写为ltl性质，就是
` G(event3 -> (X event4)) & G(event3 -> G(! event1 || ! event2))`

用ltl2tgba的命令表示
```
 ltl2tgba -D -M 'G(event3 -> (X event4)) & G(event3 -> G(! event1 || ! event2))' -d |dot -Tpdf > simpleevent.pdf

  ltl2tgba -D -M G(event3 -> (X event4)) & G(event3 -> G(! event1 || ! event2))' -H > simpleevent.hoa
```

先验证简单的性质,
event3发生后，event4立即发生并且event3同时不会发生。
```
ltl2tgba -D -M 'G(event3 -> X (event4 &  ! event3))' -d |dot -Tpdf > simpleevent.pdf
```

