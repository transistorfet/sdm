
; We are assuming we are a basic Wizard user

(define core (#0:%clone 1 (lambda ()
	(define this.name "core")
	(define this.title "Core Object")

	(define this:tell (lambda ()
		"Do Nothing."
	))

	(define this:move %thing_move)

	(define find (lambda (name)
		(if (equals? name "me")
			this
			(if (equals? name "here")
				this.location
				; TODO otherwise, search the current user for an object and then search the user's location for an object
			)
		)
	))

	(define this:acceptable (lambda (obj)
		; TODO should this be a lambda on the location?
		(this:notify-room obj
			"You try to get into $obj.name but fail miserably."
			"$user.name tries to get into $obj.name but fails miserably."
			nil)
	))
)))

(define wizard (architect:%clone 2 (lambda ()
	(define this.name "wizard")
	(define this.title "a powerful wizard")
)))

(define architect (user:%clone 3 (lambda ()
	(define this.name "generic-architect")
	(define this.title "an architect")

	(define this:@eval (lambda ()
		(eval argstr)
	))

	(define this:%save_all %thing_save_all)
)))

(define ChanServ (core:%clone 4 (lambda ()
	(define this.name "ChanServ")
	(define this.title "ChanServ")

	(define this.list (array))

	; TODO you should add functions to add and remove channels to the list??

	(define this:quit (lambda ()
		(this.list:foreach (lambda (cur)
			(define channel cur)
			(cur:leave)
		))
	))
)))

(define channel (core:%clone 5 (lambda ()
	(define this.name "generic-channel")
	(define this.title "Generic Channel")

	(define this:join (lambda ()
		(if (= (this.users:search user) -1)
			(begin
				(this.users:push user)
				(this.users:foreach (lambda (cur)
					(cur:notify N/JOIN user channel "")
				))
			)
		)
	))

	(define this:leave (lambda ()
		(if (!= (this.users:search user) -1)
			(begin
				(this.users:foreach (lambda (cur)
					(cur:notify N/LEAVE user channel "")
				))
				(this.users:remove user)
			)
		)
	))

	(define this:quit (lambda ()
		(if (!= (this.users:search user) -1)
			(begin
				; TODO there should only be one quit message per user but we can only control the channels quit...
				;(this.users:foreach (lambda (cur)
				;	(cur:notify N/QUIT user channel "")))
				(this.users:remove user)
			)
		)
	))

	(define this:say (lambda (text)
		(this.users:foreach (lambda (cur)
			(cur:notify N/SAY user channel text)
		))
	))

	(define this:emote (lambda (text)
		(this.users:foreach (lambda (cur)
			(cur:notify N/EMOTE user channel text)
		))
	))

	(define this:names (lambda ()
		(define names "")
		(this.users:foreach (lambda (cur)
			(set names (concat names cur.name " "))
		))
		(chop names)
	))

	(define this:command (lambda (text)
		(user:notify N/STATUS nil channel "Commands are not supported in this channel.")
	))
)))

(define realm (channel:%clone 6 (lambda ()
	(define this.name "#realm")
	(define this.title "TheRealm")

	(define this:join (lambda ()
		(super join)
		(cryolocker:fetch user)
	))

	(define this:leave (lambda ()
		(super leave)
		(cryolocker:store user)
	))

	(define this:quit (lambda ()
		(super quit)
		(@cryolocker:store user)
	))

	(define this:say (lambda (text)
		(user.location:say text)
	))

	(define this:emote (lambda (text)
		(user.location:emote text)
	))

	(define this:command %parse_command)
)))

(define thing (core:%clone 8 (lambda ()
	(define this.name "generic-thing")
	(define this.title "something")
	(define this.description "You're not sure what it is.")

	(define this.looked_at "$user.name examines $this.name.")

	(define this:look_self (lambda ()
		(user:notify N/STATUS nil channel (format "<yellow>$this.title"))
		(user:notify N/STATUS nil channel (format "<lightgreen>$this.description"))
	))

	(define this:print_view (lambda ()
		(user:notify N/STATUS nil channel (format "<b><lightblue>You see $this.name here."))
	))
)))

(define mobile (thing:%clone 9 (lambda ()
	(define this.name "generic-mobile")
	(define this.title "a creature")

	(define this.self_look_at "$this.name looks at himself, quizically.")
	(define this.look_at "$user.name looks at you, menicingly.")
	(define this.looked_at "$user.name looks at $this.name.")

	(define this:init (lambda ()
		(define this.hp 30)
		; TODO set all sorts of important values specific to a particular mobile
	))
)))

(define user (mobile:%clone 7 (lambda ()
	(define this.name "user")
	(define this.title "someone")

	(define this:notify %user_notify)

	(define this:tell (lambda (text)
		(this:notify N/STATUS nil channel text)
	))
)))

(define room (thing:%clone 10 (lambda ()
	(define this.name "generic-room")
	(define this.title "someplace")

	(define this.looked_at "$user.name looks around the room.")

	(define this:look (lambda (name)
		(if name
			(begin
				(define obj (user:find name))
				(if (null? obj)
					(user:tell "You don't see that here.")
					(obj:look_self)
				)
			)
			(this:look_self)
		)
	))

	(define this:look_self (lambda ()
		(user:notify N/STATUS nil channel (format "<yellow>$this.title"))
		(user:notify N/STATUS nil channel (format "<lightgreen>$this.description"))
		(user.location.contents:foreach (lambda (cur)
			(user:notify N/STATUS nil channel (format "<blue>$cur.title"))
		))
	))

	(define this:say (lambda (text)
		(this.contents:foreach (lambda (cur)
			(cur:notify N/SAY user channel text)
		))
	))

	(define this:emote (lambda (text)
		(this.contents:foreach (lambda (cur)
			(cur:notify N/EMOTE user channel text)
		))
	))

	;;; Returns 1 if the given object is allowed to enter or 0 otherwise
	(define this:acceptable (lambda (obj)
		(not this.locked)
	))

	(define this:go (lambda (dir)
		(try
			(define exit (this.exits:get dir))
		(catch (e)
			(user:tell "You can't go that way.")
		))
		(exit.invoke user)
	))

	(define this:n (lambda () (this:go "north")))
	(define this:s (lambda () (this:go "south")))
	(define this:e (lambda () (this:go "east")))
	(define this:w (lambda () (this:go "west")))
	(define this:u (lambda () (this:go "up")))
	(define this:d (lambda () (this:go "down")))
)))

(define exit (thing:%clone 11 (lambda ()
	(define this.name "generic-exit")
	(define this.title "some exit")

	(define this:invoke (lambda (name)
		(define start what.location)
		(if (and this.dest.unlocked (this.dest:acceptable what))
			; TODO does this need to be protected somehow so that somebody can't bypass it by typing "move #14"???
			(what:move this.dest)
			(if (!eqv? what.location start)
				(start:tell_all (expand-refs this.msg_leave))
				; TODO print fail message?? (optional) "So and So tries to goes east but critically fails."
			)
			(if (eqv? what.location this.dest)
				; TODO print the join messages to what.location
				; TODO print an error message to the object's location so at least they know something wrong happened?? (assuming
				;	that this is the only way anybody in that object will know that someone entered)
			)
			(user:tell (expand-refs this.msg_user_success))
		)
	))
)))

(define cryolocker (core:%clone 12 (lambda ()
	(define this.name "cryolocker")
	(define this.title "The Cryolocker")

	(define this:store (lambda (name)
		(if (!eqv? user.location this)
			(begin
				(define user.last_location user.location)
				(user:move this)
			)
		)
	))

	(define this:fetch (lambda (name)
		(if (eqv? user.location this)
			(user:move user.last_location)
		)
	))
)))

; TODO do you then have to add this to the list of channels???
(define help (channel:%clone 13 (lambda ()
	(define this.name "#help")
	(define this.title "Help Channel")
	(define this.topic "Welcome to the Help Channel")
)))

(define NickServ (core:%clone 14 (lambda ()
	(define this.name "NickServ")
	(define this.title "NickServ")

	(define this.list (array this ChanServ wizard))

	(define this:make_guest (lambda ()
		(define guest (this:next_guest))
		(define guest.name name)
		guest
	))
)))




(define start-room (room:%clone 50 (lambda ()
	(define this.name "start")
	(define this.title "The Test Room")
	(define this.description "You are in a room with loose wires and duct tape everywhere. You fear that if you touch anything, it might break.")
)))

(define transistor (wizard:%clone 100 (lambda ()
	(define this.name "transistor")
	(define this.title "A Powerful Wizard")
	(define this.description "You see an old man dressed in a flowing blue robe wearing a large pointy blue hat with white stars on it. His bushy white beard reaches half way down his chest.")
)))

(NickServ:register transistor)
(transistor:move start-room)
(cryolocker:store transistor)


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
(define ed (channel:%clone 15 (lambda ()
	(define this.name "#ed")
	(define this.title "The Editor")

	; TODO the biggest problem with this is keeping track of the editing in progress for each user

)))






;(define /core/mobile/bot (make-thing /core/mobile (lambda ()
;	(define this:init (lambda ()
;		(super:init)
;		; this is a possible idea of how to set up a timer such that the lambda is called every 30 seconds
;		(define this.tick_timer (make-timer 30 (lambda () (this:do_tick)))
;	))
;)))


;(define /core/mobile/bot/beast (make-thing /core/mobile/bot (lambda ()
;	(define this.name "beast")
;	(define this.description "You see a perfectly normal beast.")
;
;	; this function is incredibly poorly written
;	(define count-beasts (lambda (loc)
;		(define n 0)
;		(foreach loc:contents cur
;			(if (equals (type cur) (type beast))		; beast could instead be a var that always points to *this*
;				(set n (+ n 1))))
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
;					(set num (- num 1)))))
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



