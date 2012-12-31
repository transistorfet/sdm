;
; Initialize a new database with a complete system with irc on port 6667
; moo --db <dir> --bare -l code/bootstrap-complete.moo
;

(debug "Loading IRC...")

(define root #0)

(if (not (defined? user))
	(throw "No user thing defined."))

(define ChanServ (root:clone))

(define ChanServ.name "ChanServ")
(define ChanServ.title "ChanServ")

(define ChanServ.db (hash))

(define ChanServ:register (lambda (this channel)
	(if (null? (this.db:get channel.name))
		(this.db:set channel.name channel)
		#f)
))

(define ChanServ:quit (lambda (this)
	(this.db:foreach (lambda (cur)
		(define channel cur)
		(cur:leave)
	))
))

(define NickServ (root:clone))

(define NickServ.name "NickServ")
(define NickServ.title "NickServ")

(define NickServ.db (hash))

; The idea here is that when a user talks to nickserv in pm, it will execute it as a ! command
(define NickServ:say (lambda (this &rest)
	((get-method this (expand "!$cmd")) this rest)
))

(define NickServ:make-guest (lambda (this name)
	(define guest (this:next-guest))
	(define guest.name name)
	guest
))

(define NickServ:register (lambda (this person)
	(if (null? (this.db:get person.name))
		(this.db:set person.name person)
		#f)
))

(define NickServ:identify (lambda (this name password)
	(define person (this.db:get name))
	(if (null? person)
		(return #f))
	(if (equal? person.name password)
		#t
		#f)
))

(define NickServ.guest-max 0)
(define NickServ.guest-list (array))

(define NickServ:set-max-guests (lambda (this num)
	; TODO adjust the number of guests in the guest list to the new number (up or down as required)
))

(define NickServ:get-next-guest (lambda (this name)
	(define %find-next (lambda (num)
		(if (> num this.guest-list.last)
			nil
			(begin
				(define guest (this.guest-list:get num))
				(if (= guest.in-use 0)
					guest
					(%find-next (+ num 1)))))
	))

	(define guest (%find-next 0))
	(define guest.in-use 1)
	guest
))

(NickServ:register NickServ)
(NickServ:register ChanServ)

(define channel (root:clone))

(define channel.name "generic-channel")
(define channel.title "Generic Channel")

(define channel:initialize (lambda (this)
	(define this.users (array))
))

; TODO maybe this should take the user object that will join, and do a permissions check to make sure we can force a join on
;	that object.
(define channel:join (chmod 0475 (lambda (this)
	(if (!= (this.users:search user) -1)
		(return #f))
	(this.users:push user)
	(this.users:foreach (lambda (cur)
		(cur:notify N/JOIN user channel "")))
	#t
)))

(define channel:leave (chmod 0475 (lambda (this)
	(if (!= (this.users:search user) -1)
		(begin
			(this.users:foreach (lambda (cur)
				(cur:notify N/LEAVE user channel "")
			))
			(this.users:remove user)
		)
	)
)))

(define channel:quit (lambda (this)
	(if (!= (this.users:search user) -1)
		(begin
			; TODO there should only be one quit message per user but we can only control the channels quit...
			;(this.users:foreach (lambda (cur)
			;	(cur:notify N/QUIT user channel "")))
			(this.users:remove user)
		)
	)
))

(define channel:say (lambda (this text)
	(this.users:foreach (lambda (cur)
		(cur:notify N/SAY user channel text)
	))
))

(define channel:emote (lambda (this text)
	(this.users:foreach (lambda (cur)
		(cur:notify N/EMOTE user channel text)
	))
))

(define channel:names (lambda (this)
	(define names "")
	(this.users:foreach (lambda (cur)
		(set! names (concat names cur.name " "))
	))
	(chop names)
))

(define channel:command (lambda (this text)
	(user:tell "Commands are not supported in this channel.")
))


; TODO this could be generic in game.moo and use a generic channel as the root (??)
(define realm (channel:clone))

(define realm.name "#realm")
(define realm.title "TheRealm")

(define realm:join (lambda (this)
	(if (not (super join))
		(return))
	(user:tell "<b>Welcome to the Realm!")
	(user:tell "<b>To enter a command, put a period (.) before it (eg. .look)")
	(this:fetch user)
))

(define realm:leave (lambda (this)
	(super leave)
	(this:store user)
))

(define realm:quit (lambda (this)
	(super quit)
	(this:store user)
))

(define realm:say (lambda (this text)
	(user.location:say text)
))

(define realm:emote (lambda (this text)
	(user.location:emote text)
))

(define realm.prepositions (array "to" "in" "from" "is" "as"))

(define realm:command (lambda (this text)
	(define argstr (remove-word text))
	(define dobjstr "")
	(define prepstr "")
	(define iobjstr "")
	(define words (parse-words text))
	(define cmd (words:shift))
	(words:foreach (lambda (cur)
		(cond
			((!= (this.prepositions:search cur) -1)
				(set! prepstr cur))
			((equal? prepstr "")
				(set! dobjstr (concat dobjstr " " cur)))
			(else
				(set! iobjstr (concat iobjstr " " cur)))
		)
	))
	(define dobj (user:find dobjstr))
	(define iobj (user:find iobjstr))
	(define method (get-method user cmd))
	(if (not (null? method))
		(method user words)
		(begin
			(set! method (get-method user.location cmd))
			(if (not (null? method))
				(method user.location words)
				(begin			
					(set! method (get-method dobj cmd))
					(if (and (not (null? dobj)) (not (null? method)))
						(method dobj words)
						(begin			
							(set! method (get-method iobj cmd))
							(if (and (not (null? iobj)) (not (null? method)))
								(method iobj words)
								(user:tell "You don't know how to do that.")
							)
						)
					)
				)
			)
		)
	)
))

(define realm:store (chmod 0475 (lambda (this name)
	(if (!eqv? user.location this)
		(begin
			(define user.last_location user.location)
			(user:move this)))
)))

(define realm:fetch (chmod 0475 (lambda (this name)
	(if (or (not (defined? user.last_location)) (null? user.last_location))
		(user:move start-room)
		(if (eqv? user.location this)
			(user:move user.last_location)
			(this:command "look")))
)))

(ChanServ:register realm)


(define user (root:clone))

(define user.name "user")
(define user.title "someone")

(define user:connect (lambda (this task)
	(if (and (defined? this.task) (not (null? this.task)))
		(this:disconnect))
	(define this.task task)
))

(define user:disconnect (lambda (this)
	;(if (defined? this.task)
	;	(this.task:purge this))
	(define this.task nil)
	(this:%save)
))

(define user:notify (lambda (this type thing channel str)
	(if (null? this.server)
		(throw "USER: User is not connected."))
	(this.server:notify this type thing channel str)
))

(define user:tell (lambda (this text)
	(this:notify N/STATUS nil channel text)
))


(define NickServ.guest (user:clone))
(define NickServ.guest.name "Guest")


(define programmer (user:clone))

(define programmer.name "programmer")
(define programmer.title "a programmer")

(define programmer:@eval (lambda (this &args)
	(eval argstr)
))



(define channel-help (channel:clone))
(define channel-help.name "#help")
(define channel-help.title "Help Channel")
(define channel-help.topic "Welcome to the Help Channel")
(ChanServ:register channel-help)




; The editor can just be a special channel.  That way it will recieve input directly from the user via the channel interface functions
; We just need to interpret the 'say' text to be a command/input
; Commands:
;	e <method>		; Start editing the named method (create new method if necessary)
;	w			; Write changes back
;	<num>			; Edit line number <num>
;	c [<line>]		; Change current line to <line>.  If no <line> given, then the current line will be replaced by the next
;				;   line said and each following line said will be inserted until a single '.' line is entered.
;	i [<line>]		; Insert a new line above the current line.  If no <line> given, then each line said will be inserted
;				;   until a single '.' line is entered.
;	s/<pat1>/<pat2>/	; Replace all occurances of pat1 with pat2

(define channel-help (channel:clone))
(define channel-ed.name "#ed")
(define channel-ed.title "The Editor")

; TODO the biggest problem with this is keeping track of the editing in progress for each user

(ChanServ:register channel-ed)



;;;
; Initial Users
;;;

(define newuser (wizard:clone))
(define newuser.name "transistor")
(define newuser.title "A Powerful Wizard")
(define newuser.description "You see an old man dressed in a flowing blue robe wearing a large pointy blue hat with white stars on it. His bushy white beard reaches half way down his chest.")
(NickServ:register newuser)

(define newuser (wizard:clone))
(define newuser.name "trans")
(define newuser.title "trans")
(define newuser.description "You see a strange man here, twitching slightly.")
(NickServ:register newuser)

(define newuser (user:clone))
(define newuser.name "trans2")
(define newuser.title "trans2")
(define newuser.description "You see a strange man here, twitching slightly.")
(NickServ:register newuser)

(define newuser (user:clone))
(define newuser.name "trans3")
(define newuser.title "trans3")
(define newuser.description "You see a strange man here, twitching slightly.")
(NickServ:register newuser)


(define botty (mobile:clone))
(define botty.name "botty")
(define botty.title "Botty the Bot")
(define botty.description "You see an abstract program.")

(define botty:notify (lambda (this type user channel text)
	(ignore
		(if (and (= type N/SAY) (!eqv? user this))
			(if (equal? text "hello")
				(channel:say (expand "Hello $user.name")))
		)
	)
))

(NickServ:register botty)
; TODO how would this work??
;(help:join botty)
;((lambda ()
;	(define user botty)
;	(#12:join)
;))

;(architect:%save_all)


;(define /core/mobile/bot/beast (make-thing /core/mobile/bot (lambda ()
;	(define this.name "beast")
;	(define this.description "You see a perfectly normal beast.")
;
;	; this function is incredibly poorly written
;	(define count-beasts (lambda (loc)
;		(define n 0)
;		(foreach loc:contents cur
;			(if (equals (type cur) (type beast))		; beast could instead be a var that always points to *this*
;				(set! n (+ n 1))))
;		n
;	))
;
;	; this function deletes a number of beast objects
;	(define kill-beasts (lambda (loc num)
;		(foreach loc:contents cur
;			(if (<= num 0)
;				0
;				(if (equals (type cur) (type beast))
;					(delete cur)
;					(set! num (- num 1)))))
;				
;	))
;
;	; this function creates a number of beast objects
;	(define make-beasts (lambda (loc num)
;		(if (<= num 0)
;			0
;			(begin
;				; create a beast, call the init function, and teleport it to the enter room
;				(make-thing /core/mobile/bot/beast
;					(this:init)
;					(this:teleport loc))
;				(make-beasts loc (- num 1))
;			)
;		)
;				
;	))
;
;	(define this.gen_timer (make-timer 10 (lambda ()
;		(if (> (count-beasts $exit-room) 10)		; if there are more than 10 beasts in the exit room
;			(kill-beasts $exit-room 8))		; then kill 8 of them
;		(if (< (count-beasts $enter-room) 2)		; if there are less than 2 beasts in the enter room
;			(make-beasts $enter-room 5))		; then create 5 of them
;	)))
;
;	(define this:do_tick (lambda ()
;		(if (< (rand 100) 20)		; There is a 20% chance we will move north
;			(this:move north))
;	))
;)))

(debug "Done")

