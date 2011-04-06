#include "MCFA.h"
#include "MCFA_internal.h"
#include "SL.h"
#include <sys/stat.h>

extern char *BOINCDIR;

void MCFA_set_boinc_dir()
{
    BOINCDIR = (char*)malloc(50 * sizeof(char));
    strcpy(BOINCDIR, "../../../");

}

void MCFA_delete_from_workunit()
{

    system ("mysql -h localhost VCP");
    system("select name from workunit>file");
    system("exit");
    
}
void MCFA_create_boinc_wu_template(char *demon, char *exe)
{
	FILE *fw;
        int i = 0;
        char tfilename[50];
        char tfile[9];
        char filename[50];

        strcpy(tfile, "wu_");

        MCFA_get_exec_name(exe,tfilename);

//        strcat("wu", filename);
        sprintf(filename,"%s%s.xml",tfile,tfilename);
        fw = fopen(filename, "w");

	for(i=0;i<2;i++){
        fprintf(fw,"<file_info>\n");
        fprintf(fw,"<number>");
        fprintf(fw,"%d",i);
        fprintf(fw,"</number>\n");
        fprintf(fw,"</file_info>\n");
	}

    fprintf(fw,"<workunit>\n");

    fprintf(fw,"\t<file_ref>\n");
    fprintf(fw,"\t\t<file_number>%d</file_number>\n",0);
    fprintf(fw,"\t\t<open_name>%s</open_name>\n","volpex");
    fprintf(fw,"\t\t<copy_file/>\n");
    fprintf(fw,"\t</file_ref>\n");
/*
    MCFA_get_exec_name(demon,tfilename);
    fprintf(fw,"\t<file_ref>\n");
    fprintf(fw,"\t\t<file_number>%d</file_number>\n",1);
    fprintf(fw,"\t\t<open_name>%s</open_name>\n",tfilename);
    fprintf(fw,"\t\t<copy_file/>\n");
    fprintf(fw,"\t</file_ref>\n");
    */

    MCFA_get_exec_name(exe,tfilename);
    fprintf(fw,"\t<file_ref>\n");
    fprintf(fw,"\t\t<file_number>%d</file_number>\n",1);
    fprintf(fw,"\t\t<open_name>%s</open_name>\n",tfilename);
    fprintf(fw,"\t\t<copy_file/>\n");
    fprintf(fw,"\t</file_ref>\n");

    fprintf(fw,"<delay_bound>600000000</delay_bound>\n");
    fprintf(fw,"<rsc_memory_bound>8e6</rsc_memory_bound>\n");
    fprintf(fw,"<rsc_fpops_bound>9e15</rsc_fpops_bound>\n");
    fprintf(fw,"<rsc_fpops_est>7e15</rsc_fpops_est>\n");
    fprintf(fw,"<rsc_disk_bound>100000000</rsc_disk_bound>\n");


    fprintf(fw,"</workunit>\n");
fclose(fw);
    char command[50];
//    sprintf(command ,"cp %s %s/templates/%s", filename, BOINCDIR,filename);
   
//    printf("%s\n",command);
    sprintf(command ,"%s/templates/%s", BOINCDIR,filename);
//    system(command);
    rename(filename, command);
}


void MCFA_create_boinc_re_template(char *exe, int numprocs)
{

    FILE *fw;
    char tfilename[50];
    char tfile[3];
    char *filename="";
    int i=0;
     char command[50];
    strcpy(tfile, "re_");

    filename = (char*) malloc (500*sizeof(char));
    MCFA_get_exec_name(exe,tfilename);

//    for(i=0;i<numprocs;i++){
//        strcpy(filename,"");
        sprintf(filename,"%s%s.xml",tfile,tfilename,i);
        fw = fopen(filename, "w");

    fprintf(fw,"<file_info>\n");
    fprintf(fw,"\t<name><OUTFILE_%d/></name>\n",0);
    fprintf(fw,"\t<generated_locally/>\n");
    fprintf(fw,"\t<max_nbytes>100000000</max_nbytes>\n");
    fprintf(fw,"\t<upload_when_present/>\n");
    fprintf(fw,"\t<url><UPLOAD_URL/></url>\n");
    fprintf(fw,"</file_info>\n");
    
    fprintf(fw,"<file_info>\n");
    fprintf(fw,"\t<name><OUTFILE_%d/></name>\n",1);
    fprintf(fw,"\t<generated_locally/>\n");
    fprintf(fw,"\t<max_nbytes>100000000</max_nbytes>\n");
    fprintf(fw,"\t<upload_when_present/>\n");
    fprintf(fw,"\t<url><UPLOAD_URL/></url>\n");
    fprintf(fw,"</file_info>\n");

    fprintf(fw,"<result>\n");
    fprintf(fw,"\t<file_ref>\n");
    fprintf(fw,"\t\t<file_name><OUTFILE_%d/></file_name>\n",0);
    fprintf(fw,"\t\t<open_name>dump_volpex</open_name>\n");
    fprintf(fw,"\t</file_ref>\n");


    fprintf(fw,"<result>\n");
    fprintf(fw,"\t<file_ref>\n");
    fprintf(fw,"\t\t<file_name><OUTFILE_%d/></file_name>\n",1);
    fprintf(fw,"\t\t<open_name>output</open_name>\n");
    fprintf(fw,"\t</file_ref>\n");


     fprintf(fw,"</result>\n");


     fflush(fw);
fclose(fw);
//     sprintf(command ,"cp %s %s/templates", filename, BOINCDIR);
//     printf("%s\n",command);
//     system(command);
    sprintf(command ,"%s/templates/%s", BOINCDIR, filename);
     rename(filename, command);
     fflush(stdout);
//    }
}



void MCFA_create_boinc_re_template1(char *exe)
{

    FILE *fw;
    char tfilename[50];
    char tfile[3];
    char filename[50];

    strcpy(tfile, "re_");

    MCFA_get_exec_name(exe,tfilename);
    sprintf(filename,"%s%s.xml",tfile,tfilename);
    fw = fopen(filename, "w");

    fprintf(fw,"<file_info>\n");
    fprintf(fw,"\t<name><OUTFILE_0/></name>\n");
    fprintf(fw,"\t<generated_locally/>\n");
    fprintf(fw,"\t<max_nbytes>100000000</max_nbytes>\n");
    fprintf(fw,"\t<upload_when_present/>\n");
    fprintf(fw,"\t<url><UPLOAD_URL/></url>\n");
    fprintf(fw,"</file_info>\n");
    fprintf(fw,"<result>\n");
    fprintf(fw,"\t<file_ref>\n");
    fprintf(fw,"\t\t<file_name><OUTFILE_0/></file_name>\n");
    fprintf(fw,"\t\t<open_name>dump_volpex</open_name>\n");
    fprintf(fw,"\t</file_ref>\n");
     fprintf(fw,"</result>\n");
     fflush(fw);
    fclose(fw);
     char command[50];
//     sprintf(command ,"cp %s %s/templates", filename, BOINCDIR);
//     printf("%s\n",command);
//     system(command);
    sprintf(command ,"%s/templates/%s", BOINCDIR, filename);
     rename(filename, command);

}


void MCFA_create_boinc_script(char *demon, char *exe, int numprocs)
{

    
    int i = 0;
    char command[500];
    char tdemon[50];
    char texe[50];
    MCFA_get_exec_name(demon,tdemon);
    chmod(exe,S_IRWXU|S_IRWXG|S_IRWXO);
    sprintf(command ,"cp %s %s/download",exe, BOINCDIR);
    printf("%s",command);
    system(command);
    MCFA_get_exec_name(exe,texe);
    chdir(BOINCDIR);
    system("pwd");
    for(i=0; i<2; i++){
    
/*    sprintf(appname,"%s",tdemon);
    sprintf(wuname,"asgn_wu_%s_%d ", texe, i);
    sprintf(wu_template, "./templates/wu_%s.xml " texe);
*/
    sprintf(command, "./bin/create_work -assign_host 248"
                                      "  -appname %s "
                                      "-wu_name asgn_wu0_%s_%d_%d "
                                      "-wu_template ./templates/wu_%s.xml "
                                      "-result_template ./templates/re_%s.xml "
                                      "-min_quorum 1 "
                                      "-target_nresults 1 volpex.%d %s \n", 
                                      tdemon, texe, i, i,texe, texe,i,  texe);
    system (command);    

    sprintf(command, "./bin/create_work -assign_host 254"
                                      "  -appname %s "
                                      "-wu_name asgn_wu1_%s_%d_%d "
                                      "-wu_template ./templates/wu_%s.xml "
                                      "-result_template ./templates/re_%s.xml "
                                      "-min_quorum 1 "
                                      "-target_nresults 1 volpex.%d %s \n",
                                      tdemon, texe, i, i,texe, texe,i,  texe);

    printf("%s",command);
    system (command);    
    

    }

system("./bin/stop");
    system( "./bin/start");
    system("./bin/status");

}


char* MCFA_get_ip(char **ip)
{
//    char *ip;
    char *ipp;
    char tip[200];
    FILE *fp;
    fp = popen("ifconfig | grep Bcast", "r");
                         /* Handle error */;
    while (fgets(tip, 200, fp) != NULL)
    ipp = strchr(tip,'1');
    strncpy(*ip,ipp,15);

    pclose(fp);

    return (ip);



}
