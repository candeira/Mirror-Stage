#
# Regular cron jobs for the mirrorstage package
#
0 4	* * *	root	[ -x /usr/bin/mirrorstage_maintenance ] && /usr/bin/mirrorstage_maintenance
