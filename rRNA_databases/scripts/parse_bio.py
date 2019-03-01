#!/usr/bin/env python
"""
parse_bio.py

Converts bio output from vsearch clustering to MCL style cluster table.

Usage:
    parse_bio.py <bio_file> <output_table>
    parse_bio.py -h | --help

Options:
      -h --help     Show this screen.
"""
from docopt import docopt
import json

def parse_bio_clusters(bio_json, out_tab):
    """ convert bio cluster format from vsearch to our syle table """
    with open(bio_json) as BIO:
        data = json.load(BIO)

    # build map from rep to other genes
    clusters = {}
    for row, column, value in data['data']:
        rep = data['rows'][row]['id']
        gene = data['columns'][column]['id']
        if rep == gene:
            # make sure a cluster exists for this rep
            clusters.setdefault(rep, [])
        else:
            # add gene to cluster for this rep
            clusters.setdefault(rep, []).append(gene)

    # write table
    with open(out_tab, 'wt') as TAB:
        for rep, other_genes in clusters.items():
            if len(other_genes) > 0:
                TAB.write("{}\t{}\n".format(rep,
                                            "\t".join(other_genes)))

if __name__ == '__main__':
    arguments=docopt(__doc__)
    parse_bio_clusters(arguments['<bio_file>'],
					   arguments['<output_table>'])
