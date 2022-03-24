#include <grpc++/grpc++.h>
#include "afs.grpc.pb.h"
#include "commonheaders.h"
#include <grpc/impl/codegen/status.h>
#include <grpcpp/impl/codegen/status_code_enum.h>
#include <chrono>
#include <thread>

using grpc::Channel;
using grpc::Status;
using grpc::StatusCode;
using grpc::ClientContext;
using grpc::ClientReader;

using afs::AFS;
using afs::CreateReq;
using afs::CreateRes;
using afs::LsReq;
using afs::LsRes;
using afs::FetchRequest;
using afs::FetchReply;
using afs::GetattrReq;
using afs::GetattrRes;
using afs::StoreReq;
using afs::StoreRes;
using afs::UnlinkReq;
using afs::UnlinkRes;
using afs::ChmodReq;
using afs::ChmodRes;
using afs::MkdirReq;
using afs::MkdirRes;
using afs::RmdirReq;
using afs::RmdirRes;

class AfsClient {
    public:
        AfsClient(std::shared_ptr<Channel> channel) : stub_(AFS::NewStub(channel)) {}

    int interval = 1000;
    int retries = 1;

    int afs_CREATE(const char* path, char cache_path[],struct fuse_file_info *fi) {
        CreateReq request;

        request.set_path(path);

        CreateRes reply;

        ClientContext context;

        int fd;

        char client_path[MAX_PATH_LENGTH];
        getLocalPath(path, cache_path, client_path);
    
        printf("path: %s\n", client_path);

        fd = open(client_path, O_CREAT | O_APPEND | O_RDWR, S_IRWXU | S_IRWXG); //changed last 3
        printf("Creating file in local cache\n");
        if (fd == -1) {
                printf("Create Error in local cache.. \n");
                return -errno;
        }

        Status status = stub_->afs_CREATE(&context, request, &reply);
        //Set file handler
        fi->fh = fd; 
        printf("**************** File handle CREATE ************: %d\n", fd);
        //add Retry
        if(status.ok()){
            long hashfile = hashfilename(path);
            // server mtime in nanoseconds
            put(hashfile, reply.time());
            std::cout << "reply time create " << reply.time() <<std::endl;
            //flush to persistent storage
            dump(cache_path);
            return reply.ack();
        } else {
            std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            return -errno;
        }
        return 0;
    }

    int afs_FETCH(const char *path, char **buf, int *size, char* cache_path)
    {
        FetchRequest request;
        request.set_path(path);
        FetchReply *reply = new FetchReply();
        bool is_ok = false;
        do
        {
            ClientContext context;
            printf("before fetch\n");
            Status status = stub_->afs_FETCH(&context, request, reply);
            printf("after fetch\n");
            if (status.ok()) {
                is_ok = true;
                retries = 1;
                interval = 1000;
                std::cout << reply->buf() <<std::endl;
                long hashfile = hashfilename(path);
                // server mtime in nanoseconds
                put(hashfile, reply->time());
                std::cout << "reply time fetch" << reply->time() <<std::endl;
                //flush to persistent storage
                dump(cache_path);
                *buf = (char *)(reply->buf()).data();
                printf("%s\n", *buf);
                *size = reply->size();
                return 0;
            }
            is_ok = false;
        } while (retry_req(is_ok));
        return -errno;
    }

    int afs_OPEN(const char *path, struct fuse_file_info *file_info, char cache_path[])
    {
            char *buf;
            int size = -2;
            int rc;
            int fd;
            int fetchNewCopy = 0;

            struct stat cacheFileInfo;
            struct stat remoteFileInfo;

            char client_path[MAX_PATH_LENGTH];
            getLocalPath(path, cache_path, client_path);
            printf("path: %s\n", client_path);

            fd = open(client_path,  O_APPEND | O_RDWR, S_IRWXU | S_IRWXG); //changed last 3 removed O_CREAT because it has to fetch if not there
            
            if(fd == -1) {
                printf("1. file does not exist: %d\n", fd);
                fetchNewCopy = 1;
                printf("2. Open in CREAT mode (will be fetched): %d\n", fd);
                fd = open(client_path, O_CREAT |  O_APPEND | O_RDWR, S_IRWXU | S_IRWXG); //changed last 3

                if(fd==-1) printf("Reopen Error - Probable PERMISSION issues\n"); 

            } else {
                printf("4. Get stats to compare time stamps\n");
                afs_GETATTR(path, &remoteFileInfo); 
                if(remoteFileInfo.st_mtime > get(path)) {
                    fetchNewCopy = 1;
                    printf("5. Stale copy - fetch new \n");
                }
            }

            if(fetchNewCopy) {
                rc = ftruncate(fd, 0);
                printf("6. ftruncate: rc: %d\n", rc);
                if(rc<0) {
                    return -errno;
                }
                rc = afs_FETCH(path, &buf, &size, cache_path);
                printf("8. Fetching new copy. rc: %d\n", rc);
                
                if (rc<0) {
                    return -ENOENT;
                }
                printf("9. Write to file: %s\n", buf);
                write(fd, buf, size);
                printf("10. fsync\n");
                fsync(fd);
                printf("9. File Size (as fetched from server and written into cache): %d\n", size);
            }
            //This is just for debug:
            lstat(client_path, &cacheFileInfo);
            printf("10. Final Size: %d\n", cacheFileInfo.st_size);
            
            //Set file handler
            file_info->fh = fd; 
            printf("**************** File handle OPEN ************: %d\n", fd);



        return 0;
    }

    int afs_READ(const char *path, char *buffer, size_t size, off_t offset,
		      struct fuse_file_info *file_info, char cache_path[]){
        
        int ret_code = 0;
        char client_path[MAX_PATH_LENGTH];
        getLocalPath(path, cache_path, client_path);

        //Just to debug - 
        struct stat info;
        lstat(client_path, &info);

        printf("reading from : %s\n", client_path);
        printf("**************** File handle READ ************: %d\n", file_info->fh);
        printf("**************** File size, offset READ ************: %d, %d\n", size, offset);

        int fd = open(client_path, O_RDONLY);
        file_info->fh = fd;
        ret_code = pread(file_info->fh, buffer, size, offset);
        printf("READ buffer from : %s\n", buffer);


        //Just to debug - 
        lstat(client_path, &info);
        if(ret_code < 0) {
            return -errno;
        }

	    return ret_code;
    }

    bool retry_req(bool is_ok)
    {
        if(is_ok || retries > 5)
        {
            printf("retry not required\n");
            retries = 1;
            interval = 1000;
            return false;
        }
        else{
            printf("retrying for the %d time\n", retries);
            retries += 1;
            int sleep_time = interval*retries;
            printf("sleeping now for : %d milliseconds\n", sleep_time);
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
            // interval *= interval;
            printf("returning from retry_req.");
            return true;
        }
    }

    int afs_GETATTR(const char *path, struct stat *stats){
        
        bool is_ok = false;
        GetattrRes reply;
        GetattrReq request;
        request.set_path(path);
        do
        {
            ClientContext context;
            printf("do-while starting\n");
            Status status = stub_->afs_GETATTR(&context, request, &reply);
            printf("stub called\n");
            if(status.ok())
            {
                is_ok = true;
                retries = 1;
                interval = 1000;
                printf("getattr success\n");
                if(reply.err() != 0){
                    std::cout << " getattr errno: " << reply.err() << std::endl;
                    return -reply.err();
                }
                memset(stats, 0, sizeof(struct stat));

                stats->st_ino = reply.ino();
                stats->st_mode = reply.mode();
                stats->st_nlink = reply.nlink();
                stats->st_uid = reply.uid();
                stats->st_gid = reply.gid();
                stats->st_size = reply.size();
                stats->st_blksize = reply.blksize();
                stats->st_blocks = reply.blocks();
                stats->st_atime = reply.atime();
                stats->st_mtime = reply.mtime();
                stats->st_ctime = reply.ctime();
                return 0;
            }
            is_ok = false;
        } while (retry_req(is_ok));
        return -errno;
    }

    int afs_TRUNCATE(const char *path, off_t size, char cache_path[])
    {
        char client_path[MAX_PATH_LENGTH];
        getLocalPath(path, cache_path, client_path);
        printf("truncating: %s\n", client_path);
        int res = truncate(client_path, size);
        if(res == -1)
            return -errno;
        return 0;
    }

    int afs_CHOWN(const char *path, uid_t uid, gid_t gid, char cache_path[])
    {
        char client_path[MAX_PATH_LENGTH];
        getLocalPath(path, cache_path, client_path);
        printf("changing ownership of: %s\n", client_path);
        int res = lchown(client_path, uid, gid);
        if(res == -1)
            return -errno;
        return 0;
    }

    int afs_UNLINK(const char *path, char cache_path[])
    {
        char client_path[MAX_PATH_LENGTH];
        getLocalPath(path, cache_path, client_path);
        printf("unlinking: %s\n", client_path);
        int res = unlink(client_path);
        // if(res == -1)
        //     return -errno;
        // return 0;
        ClientContext context;
        UnlinkReq req;
        req.set_path(path);
        UnlinkRes reply;

        Status status = stub_->afs_UNLINK(&context, req, &reply);
        if (status.ok()) {
            return 0;
        } else {
            return -reply.error();
        }
    }

    int afs_UTIMENS(const char *path, const struct timespec ts[2], char cache_path[])
    {
        char client_path[MAX_PATH_LENGTH];
        getLocalPath(path, cache_path, client_path);
        printf("utimens: %s\n", client_path);
        int res = utimensat(0, client_path, ts, AT_SYMLINK_NOFOLLOW);
        if(res == -1)
            return -errno;
        return 0;
    }

    int afs_CHMOD(const char *path, mode_t mode, char cache_path[])
    {
        char client_path[MAX_PATH_LENGTH];
        getLocalPath(path, cache_path, client_path);
        printf("changing permissions of: %s\n", client_path);
        int res = chmod(client_path, mode);
        // if(res == -1)
        //     return -errno;
        // return 0;
        ClientContext context;
        ChmodReq req;
        req.set_path(path);
        req.set_mode(mode);
        ChmodRes reply;

        Status status = stub_->afs_CHMOD(&context, req, &reply);
        if (status.ok()) {
            return 0;
        } else {
            return -reply.error();
        }
    }
    

    int afs_MKDIR(const char *path, mode_t mode, char cache_path[])
    {
        try{
            char client_path[MAX_PATH_LENGTH];
            getLocalPath(path, cache_path, client_path);
            printf("creating directory: %s\n", client_path);

            ClientContext context;
            MkdirReq req;
            req.set_path(path);
            req.set_mode(mode);
            MkdirRes reply;
            Status status = stub_->afs_MKDIR(&context, req, &reply);
            if (status.ok()) {
                printf("error : %d\n", errno);
                    return 0;
            } else {
                printf("error while creating dir : %d\n", reply.error());
                // printf("error message : %s\n", status.error_message().c_str());
                printf("error code : %d\n", status.error_code());
                return -reply.error();
            }

            int res = mkdir(client_path, mode);
            // if(res == -1)
            //     return -errno;
            // return 0;
   

            
        }
        catch(const std::exception& e)
        {
            printf("caught in client \n");
            throw;
        }
        
    }

    int afs_RMDIR(const char *path, char cache_path[])
    {
        char client_path[MAX_PATH_LENGTH];
        getLocalPath(path, cache_path, client_path);
        printf("removing directory: %s\n", client_path);
        int res = rmdir(client_path);
        // if(res == -1)
        //     return -errno;
        // return 0;
        ClientContext context;
        RmdirReq req;
        req.set_path(path);
        RmdirRes reply;

        Status status = stub_->afs_RMDIR(&context, req, &reply);
        if (status.ok()) {
            return 0;
        } else {
            return -reply.error();
        }
    }

    int afs_STORE(const char *path, char *buf, int size, char* cache_path)
    {
        StoreReq request;
        request.set_path(path);
        request.set_size(size);
        printf("Store request: Path = %s Size = %d \n", path, size);
        request.set_buf(std::string(buf, size));
        StoreRes reply;
        bool is_ok = false;
        do
        {
            ClientContext context;
            Status status = stub_->afs_STORE(&context, request, &reply);
            if (status.ok()) {
                is_ok = true;
                retries = 1;
                interval = 1000;
                long hashfile = hashfilename(path);
                // server mtime in nanoseconds
                put(hashfile, reply.time());
                std::cout << "reply time store" << reply.time() <<std::endl;
                //flush to persistent storage
                dump(cache_path);
                return reply.error();
            }
            is_ok = false;
        } while (retry_req(is_ok));
        return -reply.error();
    }
    
    int afs_RELEASE(const char *path, struct fuse_file_info *fi, char cache_path[])
    {
        int rc = 0;
        char *buffer;
        struct stat info, remoteFileInfo;
        char client_path[MAX_PATH_LENGTH];
        getLocalPath(path, cache_path, client_path);
        lstat(client_path, &info);
        long modified = get(path) ;
        //SEMANTICS: ALWAYS FLUSH IF MODIFIED, IRRESPECTIVE OF SERVER ATTRIBUTES
        std::cout << "Modified? time elapsed - " << modified << std::endl;
        if (modified == -12345678){ //write modification
            rc = close(fi->fh);
            buffer = (char *)malloc(info.st_size);
            int fd = open(client_path,  O_APPEND | O_RDWR, S_IRWXU | S_IRWXG); 
            read(fd, buffer, info.st_size);
            afs_STORE(path, buffer, info.st_size, cache_path);
            printf("~~~~~~~~Wrote temp to main and flushed:: %s\n", buffer);
            free(buffer);
            
        }
        
        return rc;
    }

    int afs_LS(const char *path, void *buf, fuse_fill_dir_t filler) {
        LsReq request;
        request.set_path(path);

        LsRes reply;

        ClientContext context;

        std::unique_ptr<ClientReader<LsRes> > reader(
            stub_->afs_LS(&context, request));

        while (reader->Read(&reply)) {
            if(reply.error()==0) {
                filler(buf, (reply.name()).c_str(), NULL, 0, static_cast<fuse_fill_dir_flags>(0));
            }
        }

        Status status = reader->Finish();

        if (status.ok()) {
            return 0;
        } else {
            return -errno;
        }
    }

    int afs_FSYNC(const char *path, int isdatasync, struct fuse_file_info *fi)
    {
        int res;
        // char client_path[MAX_PATH_LENGTH];
        // getLocalPath(path, cache_path, client_path);
        // printf("changing permissions of: %s\n", client_path);
        printf("isdatasync : %d\n", isdatasync);
        if (isdatasync)
            res = fdatasync(fi->fh);
        else
            res = fsync(fi->fh);
        if (res == -1)
            return -errno;

        return 0;
    }


    int afs_WRITE(const char *path, const char *buffer, size_t size, off_t offset,
                      struct fuse_file_info *file_info, char cache_path[]){
        int ret_code = 0;
        

        if (size>0){
            char client_tmp_path[MAX_PATH_LENGTH];
            getLocalTmpPath(path, cache_path, client_tmp_path);
            char client_path[MAX_PATH_LENGTH];
            getLocalPath(path, cache_path, client_path);
            int fd = open(client_tmp_path, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG);

            //copy contents of old file
            struct stat info;
            lstat(client_path, &info);
            int fd_main = open(client_path,  O_APPEND | O_RDWR, S_IRWXU | S_IRWXG); 
            char *buf = (char *)malloc(info.st_size);
            read(fd_main, buf, info.st_size);
            write(fd, buf, info.st_size);
            close(fd_main);
            // Copy from file to tmp file
        //     struct stat stat;

        //    if (fstat(file_info->fh, &stat) == -1) {
        //        perror("fstat");
        //        return -1;
        //    }

        //    off64_t len, ret;
        //    len = stat.st_size;
        //    printf("leeeengthhhhhh");
        //    std::cout << len <<std::endl;

        //    do {
        //        ret = copy_file_range(file_info->fh, NULL, fd, NULL, len, 0);
        //        if (ret == -1) {
        //            perror("copy_file_range");
        //            return -1;
        //        }

        //        len -= ret;
        //    } while (len > 0 && ret > 0);

        //    printf("offfsetttttt");
        //    std::cout << offset <<std::endl;

            ret_code = pwrite(fd, buffer, size, offset); 
            fsync(fd);
            close(fd);
            remove(client_path);
            rename(client_tmp_path, client_path);  
            long hashfile = hashfilename(path);
            // server mtime in nanoseconds
            put(hashfile, -12345678); //-12345678 is dummy flag indicating a modification
            std::cout << "updating write time  " << -12345678 <<std::endl;
            //flush to persistent storage
            dump(cache_path);  

            if (ret_code == -1)
                ret_code = -errno;
            }
            
        return ret_code;
    }

    private:
        std::unique_ptr<AFS::Stub> stub_;
};