#include "../include/outputformats.hpp"

using namespace std;


/// output Blast-like alignments (code modified from SSW-library)
void report_blast (ofstream &fileout,
                s_align* a,
                char* read_name,
                char* read_seq,
                char* read_qual,
                char* ref_name,
                char* ref_seq,
                double evalue,
                uint32_t readlen,
                uint32_t bitscore,
                bool strand, // 1: forward aligned ; 0: reverse complement aligned
                uint32_t format, // 0: Blast-like human readable, 1: Blast tabular m8
                double id,
                uint32_t mismatches,
                uint32_t gaps,
                bool cigar_out /// optional flag to output CIGAR string in last column of blast m8 output
                )
{
    char to_char[5] = {'A','C','G','T','N'};

    /// Blast-like human readable alignments
    if ( format == 0 )
    {
        fileout << "Sequence ID: ";
        char* tmp = ref_name;
        while (*tmp != '\n') fileout << *tmp++;
        fileout << endl;
        
        fileout << "Query ID: ";
        tmp = read_name;
        while (*tmp != '\n') fileout << *tmp++;
        fileout << endl;
        
        fileout << "Score: " << a->score1 << " bits (" << bitscore << ")\t";
        fileout.precision(3);
        fileout << "Expect: " << evalue << "\t";
        if (strand) fileout << "strand: +\n\n";
        else fileout << "strand: -\n\n";
        if (a->cigar) {
            uint32_t i, c = 0, left = 0, e = 0, qb = a->ref_begin1, pb = a->read_begin1; //mine
            while (e < a->cigarLen || left > 0) {
                
                int32_t count = 0;
                int32_t q = qb;
                int32_t p = pb;
                fileout << "Target: ";
                fileout.width(8);
                fileout << q+1 << "    ";
                for (c = e; c < a->cigarLen; ++c) {
                    uint32_t letter = 0xf&*(a->cigar + c);
                    uint32_t length = (0xfffffff0&*(a->cigar + c))>>4;
                    uint32_t l = (count == 0 && left > 0) ? left: length;
                    for (i = 0; i < l; ++i) {
                        if (letter == 1) fileout << "-";
                        else {
                            fileout << to_char[(int)*(ref_seq + q)];
                            ++q;
                        }
                        ++ count;
                        if (count == 60) goto step2;
                    }
                }
            step2:
                fileout << "    " << q << "\n";
                fileout.width(20);
                fileout << " ";
                q = qb;
                count = 0;
                for (c = e; c < a->cigarLen; ++c) {
                    uint32_t letter = 0xf&*(a->cigar + c);
                    uint32_t length = (0xfffffff0&*(a->cigar + c))>>4;
                    uint32_t l = (count == 0 && left > 0) ? left: length;
                    for (i = 0; i < l; ++i){
                        if (letter == 0) {
                            if ((char)to_char[(int)*(ref_seq + q)] == (char)to_char[(int)*(read_seq + p)]) fileout << "|";
                            else fileout << "*";
                            ++q;
                            ++p;
                        } else {
                            fileout << " ";
                            if (letter == 1) ++p;
                            else ++q;
                        }
                        ++ count;
                        if (count == 60) {
                            qb = q;
                            goto step3;
                        }
                    }
                }
            step3:
                p = pb;
                fileout << "\nQuery: ";
                fileout.width(9);
                fileout << p+1 << "    ";
                count = 0;
                for (c = e; c < a->cigarLen; ++c) {
                    uint32_t letter = 0xf&*(a->cigar + c);
                    uint32_t length = (0xfffffff0&*(a->cigar + c))>>4;
                    uint32_t l = (count == 0 && left > 0) ? left: length;
                    for (i = 0; i < l; ++i) {
                        if (letter == 2) fileout << "-";
                        else {
                            fileout << (char)to_char[(int)*(read_seq + p)];
                            ++p;
                        }
                        ++ count;
                        if (count == 60) {
                            pb = p;
                            left = l - i - 1;
                            e = (left == 0) ? (c + 1) : c;
                            goto end;
                        }
                    }
                }
                e = c;
                left = 0;
            end:
                fileout << "    " << p << "\n\n";
            }
        }
    }
    /// Blast tabular m8
    else if ( format == 1 )
    {
        /// (1) Query
        while ((*read_name != ' ') && (*read_name != '\n') && (*read_name != '\t')) fileout << (char)*read_name++;
        /// (2) Subject
        fileout << "\t";
        while ((*ref_name != ' ') && (*ref_name != '\n') && (*ref_name != '\t')) fileout << (char)*ref_name++;
        /// (3) %id
        fileout << id << "\t";
        /// (4) alignment length
        fileout << (a->read_end1 - a->read_begin1 +1 ) << "\t";
        /// (5) mismatches
        fileout << mismatches << "\t";
        /// (6) gap openings
        fileout << gaps << "\t";
        /// (7) q.stat
        fileout << a->read_begin1+1 << "\t";
        /// (8) q.end
        fileout << a->read_end1+1 << "\t";
        /// (9) s.start
        fileout << a->ref_begin1+1 << "\t";
        /// (10) s.end
        fileout << a->ref_end1+1 << "\t";
        /// (11) e-value
        fileout << evalue << "\t";
        /// (12) bit score
        fileout << bitscore;
        /// (13) optional: output CIGAR 
        if ( cigar_out )
        {
            fileout << "\t";
            /// masked region at beginning of alignment
            if ( a->read_begin1 != 0 ) fileout << a->read_begin1 << "S";
            for (int c = 0; c < a->cigarLen; ++c) {
                uint32_t letter = 0xf&*(a->cigar + c);
                uint32_t length = (0xfffffff0&*(a->cigar + c))>>4;
                fileout << length;
                if (letter == 0) fileout << "M";
                else if (letter == 1) fileout << "I";
                else fileout << "D";
            }
            
            uint32_t end_mask = readlen-(a->read_end1-a->read_begin1)-(a->read_begin1)+1;
            /// output the masked region at end of alignment
            if ( end_mask > 0 ) fileout << end_mask << "S";
        }
        fileout << "\n";

    }//~blast tabular m8
    else
    {
        fprintf(stderr,"\n  %sERROR%s: format can only be 0 or 1 (internal code error, outputformat.cpp).\n","\033[0;31m","\033[0m");
        exit(EXIT_FAILURE);
    }
    
    
    return ;
}

/// output SAM alignments (code modified from SSW-library)
void report_sam (ofstream &fileout,
                s_align* a,
                char* read_name,
                char* read_seq,
                char* read_qual,
                char* ref_name,
                char* ref_seq,
                double evalue,
                uint32_t readlen,
                uint32_t bitscore,
                bool strand, // 1: forward aligned ; 0: reverse complement aligned
                uint32_t diff )
{
    char to_char[5] = {'A','C','G','T','N'};
    
    /// (1) Query
    while ((*read_name != ' ') && (*read_name != '\n') && (*read_name != '\t')) fileout << (char)*read_name++;
    /// set the read name ptr to end of read name
    while (*read_name != '\n') read_name++;
    
    uint32_t c;
    
    /// (2) flag
    if (!strand) fileout << "\t16\t";
    else fileout << "\t0\t";
    
    /// (3) Subject
    while ((*ref_name != ' ') && (*ref_name != '\n') && (*ref_name != '\t')) fileout << (char)*ref_name++;
    
    /// (4) Ref start
    fileout << "\t" << a->ref_begin1+1;
    
    /// (5) mapq
    fileout << "\t" << 255 << "\t";
    
    /// (6) CIGAR
    /// output the masked region at beginning of alignment
    if ( a->read_begin1 != 0 ) fileout << a->read_begin1 << "S";
    
    /// alignment length
    uint32_t len_align = 0;
    
    for (c = 0; c < a->cigarLen; ++c) {
        uint32_t letter = 0xf&*(a->cigar + c);
        uint32_t length = (0xfffffff0&*(a->cigar + c))>>4;
        fileout << length;
        if (letter == 0) { fileout << "M"; len_align+=length; }
        else if (letter == 1) { fileout << "I"; len_align+=length; }
        else fileout << "D";
    }
    
    uint32_t end_mask = readlen-(a->read_begin1)-len_align;
    /// output the masked region at end of alignment
    if ( end_mask > 0 ) fileout << end_mask << "S";
    
    /// (7) RNEXT, (8) PNEXT, (9) TLEN
    fileout << "\t*\t0\t0\t";
    
    /// (10) SEQ
    char* ptr_read_seq = read_seq;
    while (*ptr_read_seq != '\n') fileout << (char)to_char[(int)*ptr_read_seq++];
    
    /// (11) QUAL
    fileout << "\t";
    
    /// reverse-complement strand
    if (read_qual && !strand)
    {
        while (*read_qual != '\n') fileout << (char)*read_qual--;
    /// forward strand
    }else if (read_qual){
        while ((*read_qual != '\n') && (*read_qual!='\0')) fileout << (char)*read_qual++;
    /// FASTA read
    } else fileout << "*";
    
    
    /// (12) OPTIONAL FIELD: SW alignment score generated by aligner
    fileout << "\tAS:i:" << a->score1;
    
    /// (13) OPTIONAL FIELD: edit distance to the reference
    fileout << "\tNM:i:" << diff << "\n";
    
    return ;

}



/// output aligned and non-aligned reads in FASTA/FASTQ format
void report_fasta (char* acceptedstrings,
                   char* ptr_filetype_or,
                   char* ptr_filetype_ar,
                   char** reads,
                   int32_t strs,
                   vector<bool>& read_hits,
                   uint32_t file_s,
                   char* finalnt
#ifdef chimera
                   ,vector<bool>& chimeric_reads,
                   char* acceptedchimeras_file
#endif
)
{
    /// for timing different processes
	double s,f;
        
    /// output accepted reads
    if ( (ptr_filetype_ar != NULL) && (fastxout_gv || chimeraout_gv) )
    {
        eprintf("    Writing aligned FASTA/FASTQ ... ");
        TIME(s);
        
        ofstream acceptedreads;
        if ( fastxout_gv ) acceptedreads.open(acceptedstrings, ios::app);
#ifdef chimera
        ofstream acceptedchimeras;
        if ( chimeraout_gv ) acceptedchimeras.open(acceptedchimeras_file, ios::app);
#endif
        /// pair-ended reads
        if ( pairedin_gv || pairedout_gv )
        {
            /// loop through every read, output accepted reads
            for ( uint32_t i = 1; i < strs; i+=4 )
            {
                char* begin_read = reads[i-1];
                
                /// either both reads are accepted, or one is accepted and pairedin_gv
                if ( (read_hits[i] || read_hits[i+1]) && pairedin_gv)
                {
                    char* end_read = NULL;
                    if ( file_s > 0 )
                    {
                        /// first read (of split-read + paired-read)
                        if ( i==1 )
                        {
                            end_read = reads[3];
                            while (*end_read != '\0') end_read++;
                        }
                        /// all reads except the last one
                        else if ( (i+4) < strs ) end_read = reads[i+3];
                        /// last read
                        else end_read = finalnt;
                    }
                    else
                    {
                        /// all reads except the last one
                        if ( (i+4) < strs ) end_read = reads[i+3];
                        /// last read
                        else end_read = finalnt;
                    }
                    
                    /// output aligned read
                    if ( fastxout_gv )
                    {
                        if ( acceptedreads.is_open() )
                        {
#ifdef chimera
                            if ( !(chimeraout_gv && chimeric_reads[i]) )
                            {
#endif
                                while ( begin_read != end_read ) acceptedreads << (char)*begin_read++;
                                if ( *end_read == '\n' ) acceptedreads << "\n";
#ifdef chimera
                            }
#endif
                        }
                        else
                        {
                            fprintf(stderr,"  %sERROR%s: file %s (acceptedstrings) could not be opened for writing.\n\n","\033[0;31m",acceptedstrings,"\033[0m");
                            exit(EXIT_FAILURE);
                        }
                    }

                    
#ifdef chimera
                    /// output read to chimeric file
                    if ( chimeraout_gv )
                    {
                        if ( acceptedchimeras.is_open() )
                        {
                            if ( chimeric_reads[i] )
                            {
                                while ( begin_read != end_read ) acceptedchimeras << (char)*begin_read++;
                                if ( *end_read == '\n' ) acceptedchimeras << "\n";
                            }
                        }
                        else
                        {
                            fprintf(stderr,"  %sERROR%s: file %s (acceptedchimeras) could not be opened for writing.\n\n","\033[0;31m",acceptedchimeras_file,"\033[0m");
                            exit(EXIT_FAILURE);
                        }
                    }
#endif
                    
                }//~the read was accepted
            }//~for all reads
        }//~if paired-in or paired-out
        /// regular or pair-ended reads don't need to go into the same file
        else
        {
            /// loop through every read, output accepted (and chimeric) reads
            for ( uint32_t i = 1; i < strs; i+=2 )
            {
                char* begin_read = reads[i-1];
                
                /// the read was accepted
                if ( read_hits[i] )
                {
                    char* end_read = NULL;
                    /// split-read and paired-read exist at a different location in memory than the mmap
                    if ( file_s > 0 )
                    {
                        /// first read (of split-read + paired-read)
                        if ( i==1 ) end_read = reads[2];
                        /// second read (of split-read + paired-read)
                        else if ( i==3 )
                        {
                            end_read = reads[3];
                            while (*end_read != '\0') end_read++;
                        }
                        /// all reads except the last one
                        else if ( (i+2) < strs ) end_read = reads[i+1];
                        /// last read
                        else end_read = finalnt;
                    }
                    /// the first (and possibly only) file part, all reads are in mmap
                    else
                    {
                        if ( (i+2) < strs) end_read = reads[i+1];
                        else end_read = finalnt;
                    }
                    
                    /// output aligned read
                    if ( fastxout_gv )
                    {
                        if ( acceptedreads.is_open() )
                        {
#ifdef chimera
                            if ( !(chimeraout_gv && chimeric_reads[i]) )
                            {
#endif
                                while ( begin_read != end_read ) acceptedreads << (char)*begin_read++;
                                if ( *end_read == '\n' ) acceptedreads << "\n";
#ifdef chimera
                            }
#endif
                        }
                        else
                        {
                            fprintf(stderr,"  %sERROR%s: file %s (acceptedstrings) could not be opened for writing.\n\n","\033[0;31m",acceptedstrings,"\033[0m");
                            exit(EXIT_FAILURE);
                        }
                    }
                    
                    
                    
#ifdef chimera
                    /// output read to chimeric file
                    if ( chimeraout_gv )
                    {
                        if ( acceptedchimeras.is_open() )
                        {
                            if ( chimeric_reads[i] )
                            {
                                while ( begin_read != end_read ) acceptedchimeras << (char)*begin_read++;
                                if ( *end_read == '\n' ) acceptedchimeras << "\n";
                            }
                        }
                        else
                        {
                            fprintf(stderr,"  %sERROR%s: file %s (acceptedchimeras) could not be opened for writing.\n\n","\033[0;31m",acceptedchimeras_file,"\033[0m");
                            exit(EXIT_FAILURE);
                        }
                    }
#endif
                } //~if read was accepted
            }//~for all reads
        }//~if not paired-in or paired-out
            
        if ( acceptedreads.is_open() ) acceptedreads.close();
#ifdef chimera
        if ( acceptedchimeras.is_open() ) acceptedchimeras.close();
#endif
        
        TIME(f);
        eprintf(" done [%.2f sec]\n", (f-s) );
        
    }//~if ( ptr_filetype_ar != NULL )
    
    
    /// output other reads
    if ( (ptr_filetype_or != NULL) && fastxout_gv )
    {
        eprintf("    Writing not-aligned FASTA/FASTQ ... ");
        TIME(s);
        ofstream otherreads ( ptr_filetype_or, ios::app );
        
        /// pair-ended reads
        if ( pairedin_gv || pairedout_gv )
        {
            /// loop through every read, output accepted reads
            for ( uint32_t i = 1; i < strs; i+=4 )
            {
                char* begin_read = reads[i-1];
                
                /// neither of the reads were accepted, or exactly one was accepted and pairedout_gv
                if ( (!read_hits[i] && !read_hits[i+1]) || ((read_hits[i] ^ (read_hits[i+1]) && pairedout_gv)) )
                {
                    if ( otherreads.is_open() )
                    {
                        char* end_read = NULL;
                        if ( file_s > 0 )
                        {
                            /// first read (of split-read + paired-read)
                            if ( i==1 )
                            {
                                end_read = reads[3];
                                while (*end_read != '\0') end_read++;
                            }
                            /// all reads except the last one
                            else if ( (i+4) < strs ) end_read = reads[i+3];
                            /// last read
                            else end_read = finalnt;
                        }
                        else
                        {
                            /// all reads except the last one
                            if ( (i+4) < strs ) end_read = reads[i+3];
                            /// last read
                            else end_read = finalnt;
                        }
                        
                        while ( begin_read != end_read ) otherreads << (char)*begin_read++;
                        if ( *end_read == '\n' ) otherreads << "\n";
                    }
                    else
                    {
                        fprintf(stderr,"  %sERROR%s: file %s could not be opened for writing.\n\n","\033[0;31m",ptr_filetype_or,"\033[0m");
                        exit(EXIT_FAILURE);
                    }
                }//~the read was accepted
            }//~for all reads
        }//~if (pairedin_gv || pairedout_gv)
        
        /// output reads single
        else
        {
            /// loop through every read, output non-accepted reads
            for ( uint32_t i = 1; i < strs; i+=2 )
            {
                char* begin_read = reads[i-1];
                
                /// the read was accepted
                if ( !read_hits[i] )
                {
                    /// accepted reads file output
                    if ( otherreads.is_open() )
                    {
                        char* end_read = NULL;
                        /// split-read and paired-read exist at a different location in memory than the mmap
                        if ( file_s > 0 )
                        {
                            /// first read (of split-read + paired-read)
                            if ( i==1 ) end_read = reads[2];
                            /// second read (of split-read + paired-read)
                            else if ( i==3 )
                            {
                                end_read = reads[3];
                                while (*end_read != '\0') end_read++;
                            }
                            /// all reads except the last one
                            else if ( (i+2) < strs ) end_read = reads[i+1];
                            /// last read
                            else end_read = finalnt;
                        }
                        /// the first (and possibly only) file part, all reads are in mmap
                        else
                        {
                            if ( (i+2) < strs) end_read = reads[i+1];
                            else end_read = finalnt;
                        }
                        
                        while ( begin_read != end_read ) otherreads << (char)*begin_read++;
                        if ( *end_read == '\n' ) otherreads << "\n";
                    }
                    else
                    {
                        fprintf(stderr,"  %sERROR%s: file %s could not be opened for writing.\n\n","\033[0;31m",ptr_filetype_or,"\033[0m");
                        exit(EXIT_FAILURE);
                    }
                }
            }//~for all reads
        }/// if (pairedin_gv || pairedout_gv)
        
        if ( otherreads.is_open() ) otherreads.close();
        
        TIME(f);
        eprintf(" done [%.2f sec]\n", (f-s) );
    }//~if ( ptr_filetype_or != NULL )
 
    return ;
}


/// output a biom table
void report_biom (char* biomfile)
{
    ofstream biomout ( biomfile, ios::in );
    
    if (biomout.is_open())
    {
        biomout << "\"id:\"null,";
        biomout << "\"format\": \"Biological Observation Matrix 1.0.0\",";
        biomout << "\"format_url\": \"http://biom-format.org/documentation/format_versions/biom-1.0.html\"";
        biomout << "\"type\": \"OTU table\",";
        biomout << "\"generated_by\": \"SortMeRNA v2.0\",";
        biomout << "\"date\": \"\",";
        biomout << "\"rows\":[";
        biomout << "\"matrix_type\": \"sparse\",";
        biomout << "\"matrix_element_type\": \"int\",";
        biomout << "\"shape\":";
        biomout << "\"data\":";
        
        biomout.close();
    }
    
    return ;
}











