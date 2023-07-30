#!/bin/bash

# This is only used by ci/debug.sh

uid=$(stat -c '%u' /host)
gid=$(stat -c '%g' /host)
# Note that on alpine, groupadd/useradd require the "shadow" package.
# We must use them in order to bypass the 256000 limit. adduser/addgroup
# are provided by busybox, and there does not seem to be an easy way to
# work around this 256000 limit.
if [ -f /etc/alpine-release ] ; then
    echo "UID_MAX 2147483647" >> /etc/login.defs
fi
groupadd -g $gid hostgroup
useradd -s /bin/bash -m -g $gid -u $uid hostuser
if [ -e /var/run/docker.sock ] ; then
    hostdockergid=$(stat -c '%g' /var/run/docker.sock)
    guestdockergroup=`(getent group $hostdockergid || :) | cut -d: -f1`
    if ! "$guestdockergroup" ; then
        guestdockergroup=hostdocker
        if [ -f /etc/alpine-release ] ; then
            addgroup -g $hostdockergid $guestdockergroup
        else
            groupadd -g $hostdockergid $guestdockergroup
        fi
    fi
    if [ -f /etc/alpine-release ] ; then
        addgroup hostuser $guestdockergroup
    else
        usermod -a -G hostdocker $guestdockergroup
    fi
fi


## touch /tmp/trampoline.sh
## chmod 755 /tmp/trampoline.sh
echo "hostuser	ALL=(ALL:ALL) NOPASSWD:ALL" > /etc/sudoers.d/hostuser

cd /host

## if ! [ -f build ] && ! [ -e build ] ; then
    ## echo "export force_build_tree=/tmp/b" >> /tmp/trampoline.sh
## fi

# if [ -f /etc/fedora-release ] ; then
    # echo "cd /host" >> /tmp/trampoline.sh
    # echo "exec bash -i" >> /tmp/trampoline.sh
# elif [ -f /etc/debian_version ] ; then
    # # echo "cd /host" >> /tmp/trampoline.sh
    # echo "exec bash -i" >> /tmp/trampoline.sh
    # echo "exec bash" >> /tmp/trampoline.sh
# fi

# if [ -f /etc/fedora-release ] ; then
# set -x
# exec su -P -l hostuser -c /tmp/trampoline.sh
# else
# set -x
# # exec su -l hostuser -c /tmp/trampoline.sh
# fi
# 
# cat <<EOF
# # NOTE: we're not sure how to give you a shell that Just Works (tm).
# EOF
# if [ -s /tmp/trampoline.sh ] ; then
# cat <<EOF
# # You might need to run the commands from /tmp/trampoline.sh:
# `cat /tmp/trampoline.sh`
# #
# EOF
# fi

cat > /etc/profile.d/99-cado-nfs-build-tree.sh <<'EOF'
export force_build_tree=/tmp/b 
echo "# NOTE: cado-nfs build tree has just been set to $force_build_tree"
echo "# NOTE: cado-nfs source tree is under /host"
EOF

# Do we want hooks in .bashrc or .bash_profile ? alpine linux doesn't
# seem to honour .bashrc for some reason.

cat >> ~hostuser/.bash_profile <<EOF
cd /host
EOF

if [ "$CI_JOB_NAME" ] ; then
cat >> ~hostuser/.bash_profile <<EOF
CI_JOB_NAME="$CI_JOB_NAME"
export CI_JOB_NAME
. ci/000-functions.sh
. ci/001-environment.sh
set +e
EOF
fi

chown hostuser ~hostuser/.bash_profile
chgrp hostgroup ~hostuser/.bash_profile

if [ "$*" ] ; then
    echo "# WARNING: script runs with \$PWD set to " ~hostuser
    exec su -l hostuser -c "$*"
else
    # just want a shell
    exec su - hostuser "$@"
fi
