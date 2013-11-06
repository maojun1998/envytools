/*
 * Copyright (C) 2013 Martin Peres
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "bios.h"

int envy_bios_parse_power_boost(struct envy_bios *bios);
int envy_bios_parse_power_cstep(struct envy_bios *bios);

struct P_known_tables {
	uint8_t offset;
	uint16_t *ptr;
	const char *name;
};

int parse_at(struct envy_bios *bios, struct envy_bios_power *power, 
	     int idx, int offset, const char ** name)
{
	struct P_known_tables p1_tbls[] = {
		{ 0x00, &power->perf.offset, "PERFORMANCE" },
		{ 0x04, &power->timing.offset, "MEMORY TIMINGS" },
		{ 0x0c, &power->therm.offset, "THERMAL" },
		{ 0x10, &power->volt.offset, "VOLTAGE" },
		{ 0x15, &power->unk.offset, "UNK" }
	};
	struct P_known_tables p2_tbls[] = {
		{ 0x00, &power->perf.offset, "PERFORMANCE" },
		{ 0x04, &power->timing_map.offset, "MEMORY TIMINGS MAPPING" },
		{ 0x08, &power->timing.offset, "MEMORY TIMINGS" },
		{ 0x0c, &power->volt.offset, "VOLTAGE" },
		{ 0x10, &power->therm.offset, "THERMAL"  },
		{ 0x18, &power->unk.offset, "UNK" },
		{ 0x20, &power->volt_map.offset, "VOLT MAPPING" },
		{ 0x30, &power->boost.offset, "BOOST" },
		{ 0x34, &power->cstep.offset, "CSTEP" }
	};
	struct P_known_tables *tbls;
	int entries_count = 0;

	if (power->bit->version == 0x1) {
		tbls = p1_tbls;
		entries_count = (sizeof(p1_tbls) / sizeof(struct P_known_tables));
	} else if (power->bit->version == 0x2) {
		tbls = p2_tbls;
		entries_count = (sizeof(p2_tbls) / sizeof(struct P_known_tables));
	} else
		return -EINVAL;

	/* either we address by offset or idx */
	if (idx != -1 && offset != -1)
		return -EINVAL;

	/* lookup the index by the table's offset */
	if (offset > -1) {
		idx = 0;
		while (idx < entries_count && tbls[idx].offset != offset)
			idx++;
	}

	/* check the index */
	if (idx < 0 || idx >= entries_count)
		return -ENOENT;
	
	/* check the table has the right size */
	if (tbls[idx].offset + 2 > power->bit->t_len)
		return -ENOENT;
	
	if (name)
		*name = tbls[idx].name;
	
	return bios_u16(bios, 
			power->bit->t_offset + tbls[idx].offset, 
			tbls[idx].ptr);
}

int envy_bios_parse_bit_P (struct envy_bios *bios, struct envy_bios_bit_entry *bit) {
	struct envy_bios_power *power = &bios->power;
	int idx = 0;

	power->bit = bit;
	while (!parse_at(bios, power, idx, -1, NULL))
		idx++;

	envy_bios_parse_power_boost(bios);
	envy_bios_parse_power_cstep(bios);

	return 0;
}

void envy_bios_print_bit_P (struct envy_bios *bios, FILE *out, unsigned mask) {
	struct envy_bios_power *power = &bios->power;
	const char *name;
	uint16_t addr;
	int ret = 0, i = 0;
	
	if (!power->bit || !(mask & ENVY_BIOS_PRINT_PERF))
		return;

	fprintf(out, "BIT table 'P' at 0x%x, version %i\n", 
		power->bit->offset, power->bit->version);

	for (i = 0; i < power->bit->t_len; i+=2) {
		ret = bios_u16(bios, power->bit->t_offset + i, &addr);
		if (!ret && addr) {
			name = "UNKNOWN";
			ret = parse_at(bios, power, -1, i, &name);
			fprintf(out, "0x%02x: 0x%x => %s TABLE\n", i, addr, name);
		}
	}
	
	fprintf(out, "\n");
}

int envy_bios_parse_power_boost(struct envy_bios *bios) {
	struct envy_bios_power_boost *boost = &bios->power.boost;
	int i, j, err = 0;

	bios_u8(bios, boost->offset + 0x0, &boost->version);
	switch(boost->version) {
	case 0x11:
		err |= bios_u8(bios, boost->offset + 0x1, &boost->hlen);
		err |= bios_u8(bios, boost->offset + 0x2, &boost->rlen);
		err |= bios_u8(bios, boost->offset + 0x3, &boost->ssz);
		err |= bios_u8(bios, boost->offset + 0x4, &boost->snr);
		err |= bios_u8(bios, boost->offset + 0x5, &boost->entriesnum);
		boost->valid = !err;
		break;
	default:
		ENVY_BIOS_ERR("Unknown BOOST table version 0x%x\n", boost->version);
		return -EINVAL;
	};

	for (i = 0; i < boost->entriesnum; i++) {
		uint16_t data = boost->offset + boost->hlen + i * (boost->rlen + (boost->snr * boost->ssz));

		uint16_t tmp;
		err |= bios_u16(bios, data + 0x0, &tmp);
		err |= bios_u16(bios, data + 0x2, &boost->entries[i].min);
		err |= bios_u16(bios, data + 0x4, &boost->entries[i].max);

		boost->entries[i].offset = data;
		boost->entries[i].pstate = (tmp & 0x01e0) >> 5;

		boost->entries[i].entries = malloc(boost->snr * sizeof(struct envy_bios_power_boost_subentry));

		for (j = 0; j < boost->snr; j++) {
			struct envy_bios_power_boost_subentry *sub = &boost->entries[i].entries[j];
			uint16_t sdata = data + boost->rlen + j * boost->ssz;

			sub->offset = sdata;
			bios_u8(bios, sdata + 0x0, &sub->domain);
			bios_u8(bios, sdata + 0x1, &sub->percent);
			bios_u16(bios, sdata + 0x2, &sub->min);
			bios_u16(bios, sdata + 0x4, &sub->max);
		}
	}

	return 0;
}

void envy_bios_print_power_boost(struct envy_bios *bios, FILE *out, unsigned mask) {
	struct envy_bios_power_boost *boost = &bios->power.boost;
	int i, j;

	if (!(mask & ENVY_BIOS_PRINT_PERF))
		return;

	fprintf(out, "BOOST table at 0x%x, version %x\n", boost->offset, boost->version);
	envy_bios_dump_hex(bios, out, boost->offset, boost->hlen, mask);
	if (mask & ENVY_BIOS_PRINT_VERBOSE) fprintf(out, "\n");

	for (i = 0; i < boost->entriesnum; i++) {
		fprintf(out, "	%i: pstate %x min %d MHz max %d MHz\n", i,
			boost->entries[i].pstate, boost->entries[i].min,
			boost->entries[i].max);
		envy_bios_dump_hex(bios, out, boost->entries[i].offset, boost->rlen, mask);
		if (mask & ENVY_BIOS_PRINT_VERBOSE) fprintf(out, "\n");


		for (j = 0; j < boost->snr; j++) {
			struct envy_bios_power_boost_subentry *sub = &boost->entries[i].entries[j];
			fprintf(stdout, "		%i: domain %x percent %d min %d max %d\n",
				j, sub->domain, sub->percent, sub->min, sub->max);
			envy_bios_dump_hex(bios, out, sub->offset, boost->ssz, mask);
			if (mask & ENVY_BIOS_PRINT_VERBOSE) fprintf(out, "\n");
		}
	}

	fprintf(out, "\n");
}

int envy_bios_parse_power_cstep(struct envy_bios *bios) {
	struct envy_bios_power_cstep *cstep = &bios->power.cstep;
	int i, err = 0;

	bios_u8(bios, cstep->offset + 0x0, &cstep->version);
	switch(cstep->version) {
	case 0x10:
		err |= bios_u8(bios, cstep->offset + 0x1, &cstep->hlen);
		err |= bios_u8(bios, cstep->offset + 0x2, &cstep->rlen);
		err |= bios_u8(bios, cstep->offset + 0x3, &cstep->entriesnum);
		err |= bios_u8(bios, cstep->offset + 0x4, &cstep->ssz);
		err |= bios_u8(bios, cstep->offset + 0x5, &cstep->snr);
		cstep->valid = !err;
		break;
	default:
		ENVY_BIOS_ERR("Unknown CSTEP table version 0x%x\n", cstep->version);
		return -EINVAL;
	};

	for (i = 0; i < cstep->entriesnum; i++) {
		uint16_t data = cstep->offset + cstep->hlen + i * cstep->rlen;

		uint16_t tmp;
		err |= bios_u16(bios, data + 0x0, &tmp);

		cstep->ent1[i].offset = data;
		cstep->ent1[i].pstate = (tmp & 0x01e0) >> 5;
		bios_u8(bios, data + 0x3, &cstep->ent1[i].index);
	}

	cstep->ent2 = malloc(cstep->snr * sizeof(struct envy_bios_power_cstep_entry2));
	for (i = 0; i < cstep->snr; i++) {
		uint16_t data = cstep->offset + cstep->hlen + (cstep->entriesnum * cstep->rlen) + (i * cstep->ssz);

		cstep->ent2[i].offset = data;
		bios_u16(bios, data + 0x0, &cstep->ent2[i].freq);
		bios_u8(bios, data + 0x2, &cstep->ent2[i].unkn[0]);
		bios_u8(bios, data + 0x3, &cstep->ent2[i].unkn[1]);
		bios_u8(bios, data + 0x4, &cstep->ent2[i].voltage);
		cstep->ent2[i].valid = (cstep->ent2[i].freq > 0);
	}

	return 0;
}

void envy_bios_print_power_cstep(struct envy_bios *bios, FILE *out, unsigned mask) {
	struct envy_bios_power_cstep *cstep = &bios->power.cstep;
	int i;

	if (!(mask & ENVY_BIOS_PRINT_PERF))
		return;

	fprintf(out, "CSTEP table at 0x%x, version %x\n", cstep->offset, cstep->version);
	envy_bios_dump_hex(bios, out, cstep->offset, cstep->hlen, mask);
	if (mask & ENVY_BIOS_PRINT_VERBOSE) fprintf(out, "\n");

	for (i = 0; i < cstep->entriesnum; i++) {
		fprintf(out, "	%i: pstate %x index %d\n", i,
			cstep->ent1[i].pstate, cstep->ent1[i].index);
		envy_bios_dump_hex(bios, out, cstep->ent1[i].offset, cstep->rlen, mask);
		if (mask & ENVY_BIOS_PRINT_VERBOSE) fprintf(out, "\n");
	}
	fprintf(out, "---\n");
	for (i = 0; i < cstep->snr; i++) {
		if (!cstep->ent2[i].valid)
			continue;

		fprintf(out, "	%i: freq %d MHz unkn[0] %x unkn[1] %x voltage %d\n",
			i, cstep->ent2[i].freq, cstep->ent2[i].unkn[0], cstep->ent2[i].unkn[1], cstep->ent2[i].voltage);
		envy_bios_dump_hex(bios, out, cstep->ent2[i].offset, cstep->ssz, mask);
		if (mask & ENVY_BIOS_PRINT_VERBOSE) fprintf(out, "\n");
	}

	fprintf(out, "\n");
}
