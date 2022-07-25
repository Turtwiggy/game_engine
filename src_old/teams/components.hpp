// is entity A hostile to entity B
// determined by:
// filter bits: stores an entity team index
// if either entity has no filter bits,
// they arn't hostile to eachother
// on the same team, not hostile
// they are on the always hostile team,
// they will check their pet-petmaster pair
// pet master: unique key that matches all corresponding pets
// e.g. torb's turret