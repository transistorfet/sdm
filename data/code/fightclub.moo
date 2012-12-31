
(define room (clone realm))

(define room:fight (lambda (target)
	
))


/*

Characters:
	title
	level
	wins
	losses
	xp
	hp
	hp-max
	state		(resting, standing, fighting, stunned, dead??)


Commands:
challenge <target>
accept <target>
	Challenge another player to a fight.  A challenge must be accepted
	before the fight will take place.

score
	Display the player's scorecard.

summon <type>
	Summon a beast into the room to fight (this is a special added feature)

Fighting:
* fighting is turn based


* if a player's hp goes below 0, they die and the fight ends with the victor
  getting points, a +1 to wins, and various other things.  Maybe you could
  express the change as vectors???
* a player who is dead cannot fight
* a player's health regenerates slowly depending on the current state
	resting is max, standing moderate, and fighting none
	regeneration time could also be based on like a constitution score
* once a player's hp is more than 0, he can fight again.




TODO:

* how would character creation work
* have cliche ability states??? (str, int, wis, dex, con, cha??)
  or have alternative ones??? What would be useful?  How will players fight
  eachother?  How will they gain in skill over time?  How will a turn work?
  will fighting be automatic or should there be player intervension with the
  ability to use stradegy, extra skills, items, etc??
* basically, should you make this the same way that you'd expect to fight in
  a mud? or something unique like growing a beast and letting it get stronger
  and training it and so on.  Or what??

*/


