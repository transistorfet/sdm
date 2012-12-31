;
; Initialize a new database with a complete system with irc on port 6667
; moo --db <dir> --bare -l code/bootstrap-complete.moo
;

(debug "Loading Core Seed...")

(define *global*.root #0)

(define *global*.root:initialize (lambda (this)
	(define this.location nil)
	(define this.contents (array))
))

(define *global*.ChanServ (root:clone))

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

(define *global*.NickServ (root:clone))

(define NickServ.name "NickServ")
(define NickServ.title "NickServ")

(define NickServ.db (hash))

; The idea here is that when a user talks to nickserv in pm, it will execute it as a ! command
(define NickServ:say (lambda (this &rest)
	((get-method this (expand "!$cmd")) this rest)
))

(define NickServ:make_guest (lambda (this name)
	(define guest (this:next_guest))
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

(define NickServ.guest_max 0)
(define NickServ.guest_list (array))

(define NickServ:set_max_guests (lambda (this num)
	; TODO adjust the number of guests in the guest list to the new number (up or down as required)
))

(define NickServ:get_next_guest (lambda (this name)
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

(NickServ:register NickServ)
(NickServ:register ChanServ)

(define *global*.channel (root:clone))

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



(define *global*.realm (channel:clone))

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

(define *global*.thing (root:clone))

(define thing.name "generic-thing")
(define thing.title "something")
(define thing.description "You're not sure what it is.")

(define thing:move (lambda (this where)
	; TODO we need to add some (try ...) protection here
	(define was (ignore this.location))
	(if (and (not (null? was)) (not (was.contents:remove this)))
		(return))
	(define this.location where)
	(where.contents:push this)
))

(define thing:look_self (lambda (this)
	(user:tell (expand "<lightgreen>$this.description"))
))

(define thing:notify (lambda (this &args)
	"Do Nothing."
))

(define thing:tell (lambda (this &args)
	"Do Nothing."
))

(define thing:tell_view (lambda (this)
	(user:tell (expand "<b><lightblue>You see $this.title here."))
))

(define thing:find (lambda (this name)
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

(define thing:match_name (lambda (this name)
	(debug ">>>>>> " (substr 0 (strlen name) this.name))
	(equal? name (substr 0 (strlen name) this.name))
))

(define thing:acceptable (lambda (this obj)
	; TODO should this be a lambda on the location?
	(this:tell-room obj
		"You try to get into $obj.name but fail miserably."
		"$user.name tries to get into $obj.name but fails miserably."
		nil)
))


(define *global*.mobile (thing:clone))

(define mobile.name "generic-mobile")
(define mobile.title "a generic-looking creature")

(define mobile.msg_look_self "$this.name looks at himself, quizically.")
(define mobile.msg_look_victim "$user.name looks at you, menicingly.")
(define mobile.msg_look_everyone "$user.name looks at $this.title")

(define mobile.hp 30)


(define *global*.user (mobile:clone))

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


(define *global*.architect (user:clone))

(define architect.name "generic-architect")
(define architect.title "an architect")

(define architect:@eval (lambda (this &args)
	(eval argstr)
))


(define *global*.wizard (architect:clone))

(define wizard.name "wizard")
(define wizard.title "a powerful wizard")




(define *global*.room (thing:clone))

(define room.name "generic-room")
(define room.title "someplace")

(define room.looked_at "$user.name looks around the room.")

(define room:initialize (lambda (this)
	(define this.locked #f)
	(define this.exits (hash))
	(define this.display_contents #t)
))

(define room:look (lambda (this &rest)
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

(define room:look_self (lambda (this)
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

(define room:obvious_exits (lambda (this)
	(define exits "")
	(this.exits:foreach (lambda (cur)
		(if cur.obvious
			(define exits (concat exits " " cur.dir)))
	))
))

(define room:exits (lambda (this)
	(user:tell "Obvious exits:")
	(this.exits:foreach (lambda (cur)
		(if cur.obvious
			(user:tell (expand "$cur.dir - $cur.target.title")))
	))
))

(define room:tell_all (lambda (this text)
	(this.contents:foreach (lambda (cur)
		(cur:tell text)
	))
))

(define room:tell_action (lambda (this you victim action)
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

(define room:say (lambda (this text)
	(this.contents:foreach (lambda (cur)
		(cur:notify N/SAY user channel text)
	))
))

(define room:emote (lambda (this text)
	(this.contents:foreach (lambda (cur)
		(cur:notify N/EMOTE user channel text)
	))
))

;;; Returns 1 if the given object is allowed to enter or 0 otherwise
(define room:acceptable (lambda (this obj)
	(not this.locked)
))


(define room:describe (lambda (this &args)
	;(if (defined? obj)
		;(define obj.description ...)

	;)
))

(define room:add-exit (lambda (this dir location)
	(define new-exit (exit:%clone (lambda (this)
		(define this.dest location)
		(define this.dir dir)
	)))
	(this.exits:set dir new-exit)
))

(define room:go (lambda (this dir)
	(try
		(define exit (this.exits:get dir))
	(catch (e)
		(user:tell "You can't go that way.")
	))
	(exit:invoke user)
))

(define room:n (lambda (this) (this:go "north")))
(define room:s (lambda (this) (this:go "south")))
(define room:e (lambda (this) (this:go "east")))
(define room:w (lambda (this) (this:go "west")))
(define room:u (lambda (this) (this:go "up")))
(define room:d (lambda (this) (this:go "down")))


(define *global*.exit (thing:%clone))

(define exit.name "generic-exit")
(define exit.title "some exit")

(define exit.act_fail (array
	"You try to go $dir but trip."
	nil
	"$what.title tries to go $dir but trips."))
(define exit.act_leave (array
	"You head $dir"
	nil
	"$what.title heads $dir"))
(define exit.act_enter (array
	"You enter from... somewhere."
	nil
	"$what.title enters from... somewhere."))

(define exit:initialize (lambda (this)
	(define this.obvious #t)
))

(define exit:invoke (lambda (this what)
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
; Initial Rooms
;;;

(define *global*.start (room:clone))
(define start.name "start")
(define start.title "The Test Room")
(define start.description "You are in a room with loose wires and duct tape everywhere. You fear that if you touch anything, it might break.")


(define lounge (room:clone))
(define lounge.name "lounge")
(define lounge.title "The Lounge")
(define lounge.description "You are in a sparse room with an old but comfy chesterfield in one corner and a coffeemaker not far away.")



(start:add-exit "north" lounge)
(lounge:add-exit "south" start)

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

