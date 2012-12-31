
TASK TEST IDEA

(define pseudoserv (root:clone))

(define pseudoserv.server (make-server 6667))
(pseudoserv.server:set_handler pseudoserv 'connection-task)

(define pseudoserv:connection-task (lambda (this driver)
	(irc-dispatch (irc-read-msg driver))
	(this:connection-task driver)	; TODO this recursive call could cause a lot of problems. I don't think tail recursion works well
))






(define irc-server (root:clone))

;; irc-server is defined

(define irc-server:initialize (lambda (this port)
	
))


;; Keep in mind that 'this' will (should) point to the root object (in this case, main-irc-server defined below, where start is called)
(define irc-server:start (lambda (this)
	(if (not (null? this.listen))
		(throw "irc-server: server is already started"))
	(define this.listen (tcp-listen this.port))
	(try
		;; TODO we are assuming that 'this:accept' will resolve to a value which contains the object and owner as well as the func
		(this.listen:set_callback this:handle)
		(catch
			(define this.listen nil))
	)
	;; TODO we could possibly replace the try and set_callback by just passing the function during the tcp-listen call
))

(define irc-server:handle (lambda (this conn)
	;; TODO I guess you have to look up conn in a table to find the target user or something.  Instead of just storing the object
	;; of the user (already logged in), you could store a hash or array that has the user (or there has to be some way to store the
	;; user name and password temporarily until logged in, as well as a way of allowing a guest connection (there doesn't need to
	;; be an object associate with a connection though if you only allow people to login or register or even to manually log in as
	;; a guest, but you just wont be able to join channels until you do something (by sending a message to a special user like
	;; nickserv... well yeah, if you don't find the connection in the table, then when you accept a message, you just filter out
	;; certain legal message formats

	;; IMPORTANT!! you could, instead of having a table, store the information in the connection object itself (MooInterface)?
	;; Heck, if we even make it a special fixed thing value, then when a connection disconnects and that value is not -1, we can
	;; directly call a disconnect function on the user object, rather than going indirectly through the server.  We could even
	;; have it call the server, or have a value pointing to the server object that is handling it, so you know which server a
	;; connection belongs to (could be really useful)
))




;; Create an instance of a server and start it (the lambda is to make the variable local
((lambda ()
	(define main-irc-server (clone irc-server 6667))
	;; TODO the major problem with this is that you need to call this function every time the system starts, so it's not truely
	;; persistent.  You could add this to some kind of method that's called on all objects at server start, which would give you
	;; lots of flexibility, but you'd still have to treat the start-case as special
	(main-irc-server:start)

	;; You could have a cron type system that is persistent in the objects, which allows for methods to be executed based on time,
	;; and all servers could have one that periodically checks for connection and restarts itself if the connection is lost.  That
	;; way, when you start the system, these will execute and lanuch everything, but also, if a problem happens and a connection is
	;; lost, it will still try to reconnect.  It would kinda be like an autoreconnect, and could possibly be made super generic

	;; You could even have auto-reconnect listening connections (at least) builtin to the interfaces system, such that when you
	;; create an interface, a bit can be set that makes it persistent, and everything is written to disk, if the bit is set, the
	;; listening parameters are recorded, otherwise nil is recorded.

	;; There is the potential for duplication-via-write-read
	;; (i need a word for that), such that when you write everything to xml, objects that are recorded as pointers, with multiple
	;; references, will be written multiple times and when read back in will result in multiple versions instead of the original
	;; one version.  This will happen with hashes, arrays, interfaces, lambdas, everything
	;; In scheme, everything is immutable, so everything only has one shared instance (at least boolean values, nil, etc, and
	;; numbers and symbols could be implemented the same way since they are constant).  Every 'value' in the system is a pointer
	;; to a complex type (equiv to MooObject).  But the issue of shared vs unshared values is never a major concern because
	;; it's a functional language.  You really need to think more along the lines of non-functional languages

	;; Will you have every interface connection store the connection values so that it can reconnect? (port and address)
))



;; What other normal socket-based server and client things might you want to implement

;; peer connection to another sdm system (RPC-like?)
;; telnet server (near-raw command line)
;; web server system:
;;	- interactive development system (visual listing of methods/props/objects, the ability to modify methods via a textarea form, etc)
;;	- command-line emulation-like system via always on ajax-like connection (possibly using external apache2, so doesn't have to be
;;	  web-based).
;;	- display status, live values (as in dgx), etc
;; FTP although i doubt i'd implement that anyways.  I guess it could be used for sending files like code, or just for sharing between users
;; Simple pipe to another command?

;; MODBUS master over serial or TCP
;; DGX master over serial or TCP


