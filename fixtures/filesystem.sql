begin;

drop table if exists filesystem;

create table filesystem
(
    name,
    mode,
    mtime,
    data
);
insert into filesystem (name, mode, mtime, data)
select name, mode, mtime, data
from fsdir('./assets');


commit;