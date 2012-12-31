
(define channel (clone realm))

(define channel:register (lambda (this)
	; TODO register the user by adding various properites to the user
	; thing such as initial money...
))

(define channel:store (lambda (this)
	; TODO do store options
	; TODO list categories of items?? pets, food, etc
	; TODO allow the purchase of pets from here (maybe some of random
	;	starting age, and as eggs and stuff
))

(define channel:stats (lambda (this)
	; TODO list stats for the user's pet
))

(define channel:challenge (lambda (this target)
	; TODO send a challenge to another user's pet
))

(define channel:accept (lambda (this target)
	; TODO accept the challenge of another user, starting a fight
))

(define channel:feed (lambda (this food)
	; TODO feed something to your pet

	; check that the pet is resting or standing or else fail
	; decrease the hunger proportional to the food's value
	; factor in any other effects of the food (eg. increased mutation)
	; (increase weight? increase/decrease health)
))


(define channel:task_fight (lambda (this target)
	; TODO do the fight and use a sleep timer to do the delay
))


/*

Premise:
Battle Pets
You are the master of a pet whom you raise and train, and fight in combat with
other player's pets in the channel.  You must feed your pet in order to keep it
alive and healthy, and your pet will age over time.  By some as-yet-unknown
method, your pet will mutate over time randomly, and various conditions will
effect this.  They might be mutations which help or hinder the pet in combat.

Perhaps in future, you could get new eggs, which hatch to make new pets, and
you can have multiple pets and so on.

Slime molds from outerspacez become contanimanated by radioactive waste on earth
when their meteor hit a nuclear waste facility.  The offspring of these mutant
slime molds now transmutate during life to form various animal-like configurations.
Through careful training, and competing, they can grow towards various different
forms, but beware, chaos is everywhere.



Important Factors:
	* what effects does age have: mutations, maturity, strength?
	* what effects does feeding have: eating certain things changes
		mutations, strength, health, health regeneration, lack of
		feeding will cause starvation
	* what effects does battle have: possible injury, increase in xp,
		skill levels/attributes/whatever???
	* what effects does no battle have: no leveling, which would effect
		mutations

	* the aging of the pet is determined by the time that the player spends
	  in the channel (there is no punishment for not being connected)
	* T_age is the time it takes for a pet to age by 1 (1 hour of idling)
	* R_mutate is the rate of mutation factor.  Every time the pet ages,
	  this rate is used against a random number to determine if a mutation
	  occurs.  This rate is adjusted over time based on many factors.
	* R_mutate is effected by: the quality of food fed, the age of the pet
	  (takes longer to mutate as the pet grows old??), also random
	  fluctuation??
	* which mutations can occur (how good or potentially bad they would be)
	  would be determined by the level/experience of the pet, so that if
	  you don't get your pet to fight, you don't get powerful mutations,
	  you get random and useless mutations
	* what would determine or effect how good or bad a mutations might be?
	  maybe the quality of what you feed it??
	* weight could have an effect on attack and dodge abilities

	* strength = training + experience (over age) + random
	* consitution = random
	* health regeneration = constitution bonus * health factor (per time)

	* how would you get things to eat?? buy them?? with what money?? what
	  would limit it??  maybe prize money for fighting (it would be magic
	  money but that's ok)

	* could you have a genetic code thing for causing minor variations in traits.
	  It could be such that if the code is changed by only one bit at a time, the
	  differences will be minor, but still different somehow (possibly unexpectedly)


Pet Properties:
	title
	level
	age
	weight
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

flee
	Cancel a fight before your pet dies.  It counts as a defeat

feed <pet> with <food>
	Feed your pet something.  Where would you get food from?  Buying it?
	Maybe also winning some or randomly getting some.


Fighting:
* a fight is a sequence of rounds (or turns), which occur every T_round amount
  of time once a fight has started (probably every 4 seconds) and ends when
  either a pet's hp goes to or below 0, or the pet flees.
* when a fight is over, there is a winner and a loser.  The winner gets prize
  money which should be in proportion to the level of the pet defeated.  The
  winner also gets xp proportional to the level of the pet defeated
* should the loser lose something too? like some xp or something?
* should there be a punishment for dying vs fleeing?  Maybe you lose xp if
  you flee vs die, but if you die, you lose some age or money or items or
  maybe even risk of permenent death

* each round starts with an initiative check to see who goes first.  The first
  pet then does an attack roll and the second pet does a block/dodge roll,
  if the modified attack roll is higher than the modified block roll, then it's
  a hit and the first pet rolls for damage.  The same then happens in reverse
  roles.
* pets have natual weapons that they evolve, like appendages and stuff, but
  could also have equipped weapons??

CHECKS:

Every 4 seconds for all pets in 'fighting' mode:
* attack roll = 1d20 + modifiers
* defend roll = 1d20 + agility bonus
* damage roll = weapon damage + strength bonus + ...

Every 1 hour for all pets not in 'fighting' or 'stunned' or 'dead' modes:
(Once pet is no longer in fighting or dead mode, the age will increase)
* increment age
* mutations check = 1d100 * R_mutate

Every 4 seconds for all pets not in 'dead' mode:
* hunger += metabolism * base_change...
* hunger check for death by starvation, etc
* hp regeneration += ?? + constitution bonus


* if a pet's hp goes below 0, they die and the fight ends with the victor
  getting points, a +1 to wins, and various other things.  Maybe you could
  express the change as vectors???
* a pet which is dead cannot fight
* a pet's health regenerates slowly depending on the current state
	resting is max, standing moderate, and fighting none
	regeneration time could also be based on like a constitution score
* once a pet's hp is more than 0, he can fight again.




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

* you could have a task for the channel which handles aging such that every
  minute, it checks each user's pets and increases their age and do all that
  stuff
* also it could check every 4 seconds or so for hp regeneration and hunger

*/


/*

Mutations:

- fire breathing
- claws
- tentacles
- scaley skin



*/

/*

Prototyping:

New user joins:
	Welcome to Battle Pets! Type 'register' to get started.

Existing user joins:
	<user> enters with <petname> in tow.




*/



