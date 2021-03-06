#include <cmd.h>

char* my_readlink(char* pathname);


// symlink targetFileName linkFileName
int my_symlink(int argc, char* argv[])
{
    result_t result = NONE;
    const int device = running->cwd->device;

    if(argc < 3)
    {
        fprintf(stderr, "symlink: missing operand\n");
        return MISSING_OPERAND;
    }

    char* target_pathname = argv[1];
    char* link_pathname = argv[2];

    // Get the inode into memory
    int target_ino = getino(device, target_pathname);
    MINODE* target_mip = iget(device, target_ino);

    // Verify that inode exists
    // Linux actually allows you to create broken links
    if(!target_mip)
    {
        result = DOES_NOT_EXIST;
        fprintf(stderr, "symlink: failed to access '%s':"
                " No such file or directory\n", target_pathname);
        goto clean_up;
    }
    // i_block is 4 * 15 = 60 bytes
    // There are 20 bytes of reserved space after i_block (KC counts 24)
    // Lets be safe and just cutoff at 60 (don't forget null char)
    else if(strlen(target_pathname) >= 60)
    {
        result = NAME_TOO_LONG;
        fprintf(stderr, "symlink: failed to create symbolic link '%s' => '%s':"
                " Name of target too long\n", link_pathname, target_pathname);
        goto clean_up;
    }

    // From link_name, get path to parent and name of child
    char* link_parent_pathname = NULL;
    char* link_child_pathname  = NULL;
    parse_path(link_pathname, &link_parent_pathname, &link_child_pathname);

    // Get parent in memory
    int link_parent_ino = getino(device, link_parent_pathname);
    MINODE* link_parent_mip = iget(device, link_parent_ino);

    // Verify that linkParent exists
    if(!link_parent_mip)
    {
        result = NO_PARENT;
        fprintf(stderr, "symlink: failed to create symbolic link '%s' => '%s':"
                " No such file or directory\n", link_pathname, target_pathname);
        goto clean_up_more;
    }
    // Verify that linkParent is a directory
    else if(!S_ISDIR(link_parent_mip->inode.i_mode))
    {
        result = PARENT_NOT_DIR;
        fprintf(stderr, "symlink: failed to access '%s':"
                " Not a directory\n", link_pathname);
        goto clean_up_more;
    }
    // Verify that linkChild does not yet exist
    else if(getino(device, link_pathname) > 0)
    {
        result = ALREADY_EXISTS;
        fprintf(stderr, "symlink: failed to create symbolic link '%s':"
                " File exists\n", link_pathname);
        goto clean_up_more;
    }

    // Make a file with the child's name in the parent directory
    int link_ino = creat_file(link_parent_mip, link_child_pathname);
    MINODE* link_mip = iget(device, link_ino);
    INODE* link_ip = &link_mip->inode; 

    print_inode(device, 2);// DEBUG
    print_inode(device, 15);// DEBUG
    print_inode(device, link_ino);// DEBUG

    // Change the file's type to link
    // The leading 4 bits of type indicate type
    // First clear its type by ANDing with 0x0FFF
    // Then set its type to link by ORing with 0xA000
    //link_ip->i_mode = (link_ip->i_mode & 0x0FFF) | 0xA000;
    link_ip->i_mode = LINK_MODE;


    // my_readlink: INPUT/OUTPUT ERROR

    // write the string target_name into the link's i_block[]
    //strcpy((char*)(link_ip->i_block), "ab");
    //link_ip->i_size = strlen(target_pathname);
    link_ip->i_size = 2; 

    link_mip->dirty = true;
    iput(link_mip);

    INODE* link_parent_ip  = &link_parent_mip->inode;

    // Set parent's last time of access to current time
    link_parent_ip->i_atime = time(0L);
    link_parent_mip->dirty = true;

clean_up_more:
    iput(link_parent_mip); 

    print_inode(device, link_ino);// DEBUG

    free(link_parent_pathname);
    free(link_child_pathname);

clean_up:
    // Move parent inode from memory to disk
    iput(target_mip); 

        if(result != NONE)
            return result;

    return SUCCESS;
}

// returns the contents of a symlink file 
char* my_readlink(char* pathname)
{
    const int device = running->cwd->device;

    // Get the inode into memory
    int ino = getino(device, pathname);
    MINODE* mip = iget(device, ino);
    INODE* ip = &mip->inode;

    char* result = NULL;

    if(!S_ISLNK(ip->i_mode))
    {
        fprintf(stderr, "my_readlink: '%s' is not a symlink", pathname);
        goto clean_up;
    }

    char* contents = (char*)(ip->i_block);
    result = (char*)malloc((strlen(contents) + 1) * sizeof(char));
    strcpy(result, contents);

clean_up:
    iput(mip);

    return result;
}
