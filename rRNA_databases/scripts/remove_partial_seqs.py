#
# This script takes as input a SILVA FASTA file with taxonomies in sequence IDs
# and outputs all sequences with a specified kingdom, e.g. "Bacteria"
#

import skbio
import click


def parse_hmmer(fasta_fp, tblout_fp, output_fp):
    """HMMER file from --tblout parameter
    """
    seqsToFilter = []
    # sequences with with alignment to another subunit other than classifed (e.g. 16S and 5S or 23S)
    with open(tblout_fp, 'r') as in_f:
        for line in in_f:
            if line.startswith("#"):
                continue
            seq_id = line.strip().split()[0]
            seqsToFilter.append(seq_id)
    # output only sequences not having partial matches to other databases
    with open(output_fp, 'w') as out_f:
        for seq in skbio.io.read(fasta_fp, format='fasta'):
            seq_id = seq.metadata['id']
            if seq_id in seqsToFilter:
                continue
            else:
                seq_de = seq.metadata['description']
                seq = str(seq).replace('U', 'T')
                out_f.write('>%s %s\n%s\n' % (seq_id, seq_de, seq))


@click.command()
@click.option('--fasta-fp', required=True,
              type=click.Path(resolve_path=True, readable=True, exists=True,
                              file_okay=True),
              help='Fasta file with taxonomy in sequence description')
@click.option('--tblout-fp', required=True,
              type=click.Path(resolve_path=True, readable=True, exists=True,
                              file_okay=True),
              help='HMMER --tblout file')
@click.option('--output-fp', required=True,
              type=click.Path(resolve_path=True, readable=True, exists=False,
                              file_okay=True),
              help='Path to write FASTA seqs for sequences without partial rRNA')
def main(fasta_fp, tblout_fp, output_fp):

    parse_hmmer(fasta_fp, tblout_fp, output_fp)


if __name__ == "__main__":
    main()
