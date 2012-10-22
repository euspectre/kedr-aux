<$! Priorities of arguments definition: 'function' > 'function.proto' > never.
$><$if .arg.name: join$><$with .arg$><$argumentSpec_comma$><$endwith: join
$><$elseif .proto.arg.name: join$><$with .proto.arg$><$argumentSpec_comma$><$endwith: join$><$endif$>