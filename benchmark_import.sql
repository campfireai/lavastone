create table if not exists authors (id integer primary key, author text not null, author_location text not null, unique (author, author_location));
create table if not exists recipes (title text, author_id integer, num_likes int, foreign key(author_id) references authors(id));

create table if not exists recipes_tmp (title text, author text, author_location text, num_likes int);

.mode csv
.import data.csv recipes_tmp

insert into authors (author, author_location)
  select distinct author, author_location from recipes_tmp;

insert into recipes select
  recipes_tmp.title,
  authors.id,
  recipes_tmp.num_likes
  from recipes_tmp left join authors
  on authors.author = recipes_tmp.author
  and authors.author_location = recipes_tmp.author_location;

drop table recipes_tmp;
