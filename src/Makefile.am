include_HEADERS = compass.h
lib_LTLIBRARIES = libcompass.la
libcompass_la_CXXFLAGS = -O3
libcompass_la_CPPFLAGS = \
-I$(srcdir) \
-I$(srcdir)/env \
-I$(srcdir)/util \
-I$(srcdir)/tsp \
-I$(srcdir)/op \
-I$(srcdir)/data

#libcompass_la_LDFLAGS = \
#-version-info 38:0:2 \
#-export-symbols-regex '^compass_*'

libcompass_la_SOURCES = \
prob.c \
data/near.c \
data/data.c \
data/edgelen-cc.c \
data/xnear.c \
data/delaunay.c \
data/kdtree/kdbuild.c \
data/kdtree/kdnear.c \
data/kdtree/kdspan.c \
data/kdtree/kdtwoopt.c \
env/alloc.c \
env/dlsup.c \
env/env.c \
env/error.c \
env/stdout.c \
env/stream.c \
env/time.c \
env/tls.c \
op/op.c \
op/io.c \
op/prob.c \
op/solution.c \
op/init/init.c \
op/init/select.c \
op/ea/crossover.c \
op/ea/drop.c \
op/ea/add.c \
op/ea/selection.c \
op/ea/ea.c \
op/ea/mutation.c \
tsp/tsp.c \
tsp/prob.c \
tsp/init/init.c \
tsp/linkern/flip_two.c \
tsp/linkern/linkern.c \
tsp/ls/ls.c \
util/dmp.c \
util/jd.c \
util/allocrus.c \
util/bgetopt.c \
util/dheaps_i.c \
util/edgemap.c \
util/edgeutil.c \
util/fastread.c \
util/safe_io.c \
util/urandom.c \
util/rng.c \
util/rng1.c \
util/util.c \
util/zeit.c \
util/genhash.c \
util/priority.c \
util/signal.c \
util/sortrus.c \
util/str2int.c \
util/str2num.c
