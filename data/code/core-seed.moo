
(debug "Loading Core Seed...")

(define *global*.root #0)

(define *global*.root:initialize (lambda (this)
	(define this.location nil)
	(define this.contents (array))
))

(define *global*.ChanServ (root:%clone (lambda (this)
	(define this.name "ChanServ")
	(define this.title "ChanServ")

	(define this.db (hash))

	(define this:register (lambda (this channel)
		(if (null? (this.db:get channel.name))
			(this.db:set channel.name channel)
			#f)
	))

	(define this:quit (lambda (this)
		(this.db:foreach (lambda (cur)
			(define channel cur)
			(cur:leave)
		))
	))
)))

(define *global*.NickServ (root:%clone (lambda (this)
	(define this.name "NickServ")
	(define this.title "NickServ")

	(define this.db (hash))

	; The idea here is that when a user talks to nickserv in pm, it will execute it as a ! command
	(define this:say (lambda (this &rest)
		((get-method this (expand "!$cmd")) this rest)
	))

	(define this:make_guest (lambda (this name)
		(define guest (this:next_guest))
		(define guest.name name)
		guest
	))

	(define this:register (lambda (this person)
		(if (null? (this.db:get person.name))
			(this.db:set person.name person)
			#f)
	))

	(define this:identify (lambda (this name password)
		(define person (this.db:get name))
		(if (null? person)
			(return #f))
		(if (equal? person.name password)
			#t
			#f)
	))

	(define this.guest_max 0)
	(define this.guest_list (array))

	(define this:set_max_guests (lambda (this num)
		; TODO adjust the number of guests in the guest list to the new number (up or down as required)
	))

	(define this:get_next_guest (lambda (this name)
		(define _find_next (lambda (num)
			(if (> num this.guest_list.last)
				nil
				(begin
					(define guest (this.guest_list:get num))
					(if (= guest.in_use 0)
						guest
						(_find_next (+ num 1)))))
		))

		(define guest (_find_next 0))
		(define guest.in_use 1)
		guest
	))
)))
(NickServ:register NickServ)
(NickServ:register ChanServ)

(define *global*.channel (root:%clone (lambda (this)
	(define this.name "generic-channel")
	(define this.title "Generic Channel")

	(define this:initialize (lambda (this)
		(define this.users (array))
	))

	; TODO maybe this should take the user object that will join, and do a permissions check to make sure we can force a join on
	;	that object.
	(define this:join (chmod 0475 (lambda (this)
		(if (!= (this.users:search user) -1)
			(return #f))
		(this.users:push user)
		(this.users:foreach (lambda (cur)
			(cur:notify N/JOIN user channel "")))
		#t
	)))

	(define this:leave (chmod 0475 (lambda (this)
		(if (!= (this.users:search user) -1)
			(begin
				(this.users:foreach (lambda (cur)
					(cur:notify N/LEAVE user channel "")
				))
				(this.users:remove user)
			)
		)
	)))

	(define this:quit (lambda (this)
		(if (!= (this.users:search user) -1)
			(begin
				; TODO there should only be one quit message per user but we can only control the channels quit...
				;(this.users:foreach (lambda (cur)
				;	(cur:notify N/QUIT user channel "")))
				(this.users:remove user)
			)
		)
	))

	(define this:say (lambda (this text)
		(this.users:foreach (lambda (cur)
			(cur:notify N/SAY user channel text)
		))
	))

	(define this:emote (lambda (this text)
		(this.users:foreach (lambda (cur)
			(cur:notify N/EMOTE user channel text)
		))
	))

	(define this:names (lambda (this)
		(define names "")
		(this.users:foreach (lambda (cur)
			(set! names (concat names cur.name " "))
		))
		(chop names)
	))

	(define this:command (lambda (this text)
		(user:tell "Commands are not supported in this channel.")
	))
)))

(define *global*.realm (channel:%clone (lambda (this)
	(define this.name "#realm")
	(define this.title "TheRealm")

	(define this:join (lambda (this)
		(if (not (super join))
			(return))
		(user:tell "<b>Welcome to the Realm!")
		(user:tell "<b>To enter a command, put a period (.) before it (eg. .look)")
		(this:fetch user)
	))

	(define this:leave (lambda (this)
		(super leave)
		(this:store user)
	))

	(define this:quit (lambda (this)
		(super quit)
		(this:store user)
	))

	(define this:say (lambda (this text)
		(user.location:say text)
	))

	(define this:emote (lambda (this text)
		(user.location:emote text)
	))

	(define this.prepositions (array "to" "in" "from" "is" "as"))

	(define this:command (lambda (this text)
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

	(define this:store (chmod 0475 (lambda (this name)
		(if (!eqv? user.location this)
			(begin
				(define user.last_location user.location)
				(user:move this)))
	)))

	(define this:fetch (chmod 0475 (lambda (this name)
		(if (or (not (defined? user.last_location)) (null? user.last_location))
			(user:move start-room)
			(if (eqv? user.location this)
				(user:move user.last_location)
				(this:command "look")))
	)))
)))
(ChanServ:register realm)

(define *global*.thing (root:%clone (lambda (this)
	(define this.name "generic-thing")
	(define this.title "something")
	(define this.description "You're not sure what it is.")

	(define this:move (lambda (this where)
		; TODO we need to add some (try ...) protection here
		(define was (ignore this.location))
		(if (and (not (null? was)) (not (was.contents:remove this)))
			(return))
		(define this.location where)
		(where.contents:push this)
	))

	(define this:look_self (lambda (this)
		(user:tell (expand "<lightgreen>$this.description"))
	))

	(define this:notify (lambda (this &args)
		"Do Nothing."
	))

	(define this:tell (lambda (this &args)
		"Do Nothing."
	))

	(define this:tell_view (lambda (this)
		(user:tell (expand "<b><lightblue>You see $this.title here."))
	))

	(define this:find (lambda (this name)
		(cond
			((equal? name "me")
				this)
			((equal? name "here")
				this.location)
			(else
				(define obj nil)
				(user.contents:foreach (lambda (cur)
					(if (cur:match_name name)
						(set! obj cur))
				))
				obj
			)
		)
	))

	(define this:match_name (lambda (this name)
		(debug ">>>>>> " (substr 0 (strlen name) this.name))
		(equal? name (substr 0 (strlen name) this.name))
	))

	(define this:acceptable (lambda (this obj)
		; TODO should this be a lambda on the location?
		(this:tell-room obj
			"You try to get into $obj.name but fail miserably."
			"$user.name tries to get into $obj.name but fails miserably."
			nil)
	))
)))

(define *global*.mobile (thing:%clone (lambda (this)
	(define this.name "generic-mobile")
	(define this.title "a generic-looking creature")

	(define this.msg_look_self "$this.name looks at himself, quizically.")
	(define this.msg_look_victim "$user.name looks at you, menicingly.")
	(define this.msg_look_everyone "$user.name looks at $this.title")

	(define this.hp 30)
)))

(define *global*.user (mobile:%clone (lambda (this)
	(define this.name "user")
	(define this.title "someone")

	(define this:connect (lambda (this task)
		(if (and (defined? this.task) (not (null? this.task)))
			(this:disconnect))
		(define this.task task)
	))

	(define this:disconnect (lambda (this)
		;(if (defined? this.task)
		;	(this.task:purge this))
		(define this.task nil)
		(this:%save)
	))

	(define this:notify %user_notify)

	(define this:tell (lambda (this text)
		(this:notify N/STATUS nil channel text)
	))
)))

(define NickServ.guest (user:%clone (lambda (this)
	(define this.name "Guest")
)))


(define *global*.architect (user:%clone (lambda (this)
	(define this.name "generic-architect")
	(define this.title "an architect")

	(define this:@eval (lambda (this &args)
		(eval argstr)
	))
)))

(define *global*.wizard (architect:%clone (lambda (this)
	(define this.name "wizard")
	(define this.title "a powerful wizard")
)))




(define *global*.room (thing:%clone (lambda (this)
	(define this.name "generic-room")
	(define this.title "someplace")

	(define this.looked_at "$user.name looks around the room.")

	(define this:initialize (lambda (this)
		(define this.locked #f)
		(define this.exits (hash))
		(define this.display_contents #t)
	))

	(define this:look (lambda (this &rest)
		(define name (rest:get 0))
		(debug "Name>> " name)
		(if (null? name)
			(this:look_self)
			(begin
				(define obj (user:find name))
				(if (null? obj)
					(user:tell "You don't see that here.")
					(obj:look_self)
				)
			)
		)
	))

	(define this:look_self (lambda (this)
		(if (= (this.contents:search user) -1)
			(user:tell "You don't have clairvoyance.")
			(begin
				(user:tell (expand "<yellow>$this.title"))
				(user:tell (expand "<lightgreen>$this.description"))
				(user:tell (expand (concat "<b>Exits:" (this:obvious_exits) ".")))
				(if this.display_contents
					(user.location.contents:foreach (lambda (cur)
						(user:tell (expand "<blue>$cur.title"))
					))
				)
			)
		)
	))

	(define this:obvious_exits (lambda (this)
		(define exits "")
		(this.exits:foreach (lambda (cur)
			(if cur.obvious
				(define exits (concat exits " " cur.dir)))
		))
	))

	(define this:exits (lambda (this)
		(user:tell "Obvious exits:")
		(this.exits:foreach (lambda (cur)
			(if cur.obvious
				(user:tell (expand "$cur.dir - $cur.target.title")))
		))
	))

	(define this:tell_all (lambda (this text)
		(this.contents:foreach (lambda (cur)
			(cur:tell text)
		))
	))

	(define this:tell_action (lambda (this you victim action)
		(this.contents:foreach (lambda (cur)
			(if (eqv? cur you)
				(cur:tell (expand (action:get 0)))
				(if (eqv? cur victim)
					(cur:tell (expand (action:get 1)))
					(cur:tell (expand (action:get 2)))
				)
			)
		))
	))

	(define this:say (lambda (this text)
		(this.contents:foreach (lambda (cur)
			(cur:notify N/SAY user channel text)
		))
	))

	(define this:emote (lambda (this text)
		(this.contents:foreach (lambda (cur)
			(cur:notify N/EMOTE user channel text)
		))
	))

	;;; Returns 1 if the given object is allowed to enter or 0 otherwise
	(define this:acceptable (lambda (this obj)
		(not this.locked)
	))


	(define this:describe (lambda (this &args)
		;(if (defined? obj)
			;(define obj.description ...)

		;)
	))

	(define this:add-exit (lambda (this dir location)
		(define new-exit (exit:%clone (lambda (this)
			(define this.dest location)
			(define this.dir dir)
		)))
		(this.exits:set dir new-exit)
	))

	(define this:go (lambda (this dir)
		(try
			(define exit (this.exits:get dir))
		(catch (e)
			(user:tell "You can't go that way.")
		))
		(exit:invoke user)
	))

	(define this:n (lambda (this) (this:go "north")))
	(define this:s (lambda (this) (this:go "south")))
	(define this:e (lambda (this) (this:go "east")))
	(define this:w (lambda (this) (this:go "west")))
	(define this:u (lambda (this) (this:go "up")))
	(define this:d (lambda (this) (this:go "down")))
)))

(define *global*.exit (thing:%clone (lambda (this)
	(define this.name "generic-exit")
	(define this.title "some exit")

	(define this.act_fail (array
		"You try to go $dir but trip."
		nil
		"$what.title tries to go $dir but trips."))
	(define this.act_leave (array
		"You head $dir"
		nil
		"$what.title heads $dir"))
	(define this.act_enter (array
		"You enter from... somewhere."
		nil
		"$what.title enters from... somewhere."))

	(define this:initialize (lambda (this)
		(define this.obvious #t)
	))

	(define this:invoke (lambda (this what)
		(define dir this.dir)
		(define start what.location)
		(if (this.dest:acceptable what)
			(begin
				; TODO does this need to be protected somehow so that somebody can't bypass it by typing "move #14"???
				(what:move this.dest)
				(if (eqv? what.location start)
					(start:tell_action user nil this.act_fail)
					(start:tell_action user nil this.act_leave)
				)
				(if (eqv? what.location this.dest)
					(what.location:tell_action user nil this.act_enter))
				(if (eqv? user what)
					(what.location:look_self))
			)
		)
	))
)))

(ChanServ:register (channel:%clone (lambda (this)
	(define this.name "#help")
	(define this.title "Help Channel")
	(define this.topic "Welcome to the Help Channel")
)))




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
(ChanServ:register (channel:%clone (lambda (this)
	(define this.name "#ed")
	(define this.title "The Editor")

	; TODO the biggest problem with this is keeping track of the editing in progress for each user

)))




;;;
; Initial Rooms
;;;

(define *global*.start-room (room:%clone 50 (lambda (this)
	(define this.name "start")
	(define this.title "The Test Room")
	(define this.description "You are in a room with loose wires and duct tape everywhere. You fear that if you touch anything, it might break.")

)))

(define lounge (room:%clone (lambda (this)
	(define this.name "lounge")
	(define this.title "The Lounge")
	(define this.description "You are in a sparse room with an old but comfy chesterfield in one corner and a coffeemaker not far away.")

)))

(start-room:add-exit "north" lounge)
(lounge:add-exit "south" start-room)

;;;
; Initial Users
;;;

(NickServ:register (wizard:%clone 100 (lambda (this)
	(define this.name "transistor")
	(define this.title "A Powerful Wizard")
	(define this.description "You see an old man dressed in a flowing blue robe wearing a large pointy blue hat with white stars on it. His bushy white beard reaches half way down his chest.")
)))

(NickServ:register (wizard:%clone 101 (lambda (this)
	(define this.name "trans")
	(define this.title "trans")
	(define this.description "You see a strange man here, twitching slightly.")
)))

(NickServ:register (user:%clone 102 (lambda (this)
	(define this.name "trans2")
	(define this.title "trans2")
	(define this.description "You see a strange man here, twitching slightly.")
)))

(NickServ:register (user:%clone 103 (lambda (this)
	(define this.name "trans3")
	(define this.title "trans3")
	(define this.description "You see a strange man here, twitching slightly.")
)))


(define botty (mobile:%clone (lambda (this)
	(define this.name "botty")
	(define this.title "Botty the Bot")
	(define this.description "You see an abstract program.")

	(define this:notify (lambda (this type user channel text)
		(ignore
			(if (and (= type N/SAY) (!eqv? user this))
				(if (equal? text "hello")
					(channel:say (expand "Hello $user.name")))
			)
		)
	))
)))
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

