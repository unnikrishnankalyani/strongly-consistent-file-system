#include <time.h>
#include <sys/stat.h>

#include <iostream>
#define FUSE_USE_VERSION 31
#include <fuse.h>


#include <grpcpp/grpcpp.h>
#include "afs.grpc.pb.h"
#include "AfsClient.h"

static struct options {
	AfsClient* afsclient;
	int show_help;
} options;

char cache_path[MAX_PATH_LENGTH];

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

static int client_create(const char* path, mode_t mode, struct fuse_file_info *fi)
{
    std::cout <<"calling client create" <<std::endl;
    return options.afsclient->afs_CREATE(path, cache_path, fi);
}
static int client_mknod(const char* path, mode_t mode, dev_t dev)
{
    std::cout <<"dummy mknod" <<std::endl;
    return 0;
}
static int client_getattr(const char *path, struct stat *stats,
             struct fuse_file_info *fi)
{
    return options.afsclient->afs_GETATTR(path, stats);
}

static int client_write(const char *path, const char *buffer, size_t size, off_t offset,
                      struct fuse_file_info *file_info)
{
    return options.afsclient->afs_WRITE(path, buffer, size, offset, file_info, cache_path);
}


static int client_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *file_info)
{
    return options.afsclient->afs_READ(path, buffer, size, offset, file_info, cache_path);

    // int ret_code = 0;

    // ret_code = pread(file_info->fh, buffer, size, offset);
    // if(ret_code < 0) {
    //     return -errno;
    // }

    // return ret_code;
}

static int client_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
    std::cout <<"readdir " <<std::endl;
    return options.afsclient->afs_LS(path, buf, filler);
}

static int client_open(const char *path, struct fuse_file_info *file_info)
{
    return options.afsclient->afs_OPEN(path, file_info, cache_path);
}

static int client_release(const char *path, struct fuse_file_info *fi)
{
    return options.afsclient->afs_RELEASE(path, fi, cache_path);
}

// static int client_mkdir(const char* path, mode_t mode)
// {
//     return options.afsclient->afs_MKDIR(path, mode);
// }

static int client_truncate(const char *path, off_t size, struct fuse_file_info *file_info)
{
    return options.afsclient->afs_TRUNCATE(path, size, cache_path);
}

static int client_chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *file_info)
{
    return options.afsclient->afs_CHOWN(path, uid, gid, cache_path);
}

static int client_utimens(const char *path, const struct timespec ts[2], struct fuse_file_info *file_info)
{
    return options.afsclient->afs_UTIMENS(path, ts, cache_path);
}

static int client_chmod(const char *path, mode_t mode, struct fuse_file_info *file_info)
{
    return options.afsclient->afs_CHMOD(path, mode, cache_path);
}

static int client_fsync(const char *path, int isdatasync, struct fuse_file_info *fi)
{
    return options.afsclient->afs_FSYNC(path, isdatasync, fi);
}

static int client_unlink(const char *path)
{
    return options.afsclient->afs_UNLINK(path, cache_path);
}

static int client_mkdir(const char *path, mode_t mode)
{
    try{
        return options.afsclient->afs_MKDIR(path, mode, cache_path);
    }
    catch(const std::exception& e)
    {
        printf("retrying for mkdir!\n");
    }
    
}

static int client_rmdir(const char *path)
{
    return options.afsclient->afs_RMDIR(path, cache_path);
}

struct client_fuse_operations:fuse_operations
{
    client_fuse_operations ()
    {
        create     = client_create;
        write      = client_write;
        read       = client_read;
        readdir    = client_readdir;
        open       = client_open;
        getattr    = client_getattr;
        //uncomment the below as and when the corresponding implementation is done.
        release    = client_release;
        // mkdir      = client_mkdir;
        mknod      = client_mknod;
        truncate   = client_truncate;
        chown      = client_chown;
        utimens    = client_utimens;
        chmod      = client_chmod;
        fsync      = client_fsync;
        // getxattr   = client_getxattr;
        unlink     = client_unlink;
        // flush      = client_flush;
        mkdir      = client_mkdir;
        rmdir      = client_rmdir;
    }
} client_oper;

int main(int argc, char* argv[]){

	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	options.afsclient = new AfsClient(grpc::CreateChannel(
  "10.10.1.1:50051", grpc::InsecureChannelCredentials()));

    //cache path and actual path
    strncpy(cache_path, realpath(argv[argc-1], NULL), MAX_PATH_LENGTH);
    printf("argv[argc-1] : %s\n", argv[argc-1]);
    // strncat(cache_path, "/", MAX_PATH_LENGTH);
    argv[argc-1] = NULL;
    argc--;
    printf("File System Cache Path on Client: %s\n", cache_path);
    //initialize the hash table to store modified time
    char client_path[MAX_PATH_LENGTH];
    char * path = "/database.txt";
    getLocalPath(path, cache_path, client_path);
    int fd = open(client_path, O_CREAT |  O_APPEND | O_RDWR, S_IRWXU | S_IRWXG); //changed last 3
    close(fd);
    init_ht();

    //read stored k,v
    read_from_database(cache_path);


    return fuse_main(argc, argv, &client_oper, NULL);
}
