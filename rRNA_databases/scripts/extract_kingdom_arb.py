#
# This script takes as input a SILVA FASTA file with taxonomies in sequence IDs
# and outputs all sequences with a specified kingdom, e.g. "Bacteria"
#

import skbio
import click


def extract_seqs(kingdom, silva_fasta_fp, output_fp):
    num_seq = 0
    total_num_seq = 0
    for seq in skbio.io.read(silva_fasta_fp, format = 'fasta'):
        seq_id = seq.metadata['id']
        seq_de = seq.metadata['description']
        seq_kingdom = seq_de.split(';')[0]
        total_num_seq += 1
        if seq_kingdom == kingdom:
            with open(output_fp, "a") as output_f:
                output_f.write('>%s %s\n%s\n' % (seq_id, seq_de, seq))
                num_seq += 1
    print("Total number of sequences: %s" % total_num_seq)
    print("Number of sequences output: %s" % num_seq)


@click.command()
@click.option('--silva-fasta-fp', required=True,
              type=click.Path(resolve_path=True, readable=True, exists=True,
                              file_okay=True),
              help='SILVA Fasta file with taxonomy in sequence description')
@click.option('--kingdom', required=True,
              type=click.Choice(['Bacteria', 'Archaea', 'Eukaryota']),
              help='Kingdom for which to extract sequences')
@click.option('--output-fp', required=True,
              type=click.Path(resolve_path=True, readable=True, exists=False,
                              file_okay=True),
              help='Path to write FASTA seqs for sequences belonging to a single kingdom')
def main(silva_fasta_fp, kingdom, output_fp):

    extract_seqs(kingdom, silva_fasta_fp, output_fp)


if __name__ == "__main__":
    main()