#ifndef _SECP256K1_BULLETPROOF_
# define _SECP256K1_BULLETPROOF_

# include "secp256k1.h"
# include "secp256k1_generator.h"
# include "secp256k1_rangeproof.h"

# ifdef __cplusplus
extern "C" {
# endif

/** Opaque structure representing a large number of NUMS generators */
typedef struct secp256k1_bulletproof_generators secp256k1_bulletproof_generators;

/** Version number used in header of circuit and circuit-assignment binary files */
#define SECP256K1_BULLETPROOF_CIRCUIT_VERSION	1

/* Maximum depth of 31 lets us validate an aggregate of 2^25 64-bit proofs */
#define SECP256K1_BULLETPROOF_MAX_DEPTH 60

/* Size of a hypothetical 31-depth rangeproof, in bytes */
#define SECP256K1_BULLETPROOF_MAX_PROOF (160 + 66*32 + 7)

/* Maximum memory, in bytes, that may be allocated to store a circuit representation */
#define SECP256K1_BULLETPROOF_MAX_CIRCUIT (1024 * 1024 * 1024)

/** Allocates and initializes a list of NUMS generators, along with precomputation data
 *  Currently `precomp_n` should always be set to 1, since precomputation is not used anywhere.
 *  Returns a list of generators, or NULL if allocation failed.
 *  Args:          ctx: pointer to a context object (cannot be NULL)
 *  In:   blinding_gen: generator that blinding factors will be multiplied by (cannot be NULL)
 *                   n: number of NUMS generators to produce
 *           precomp_n: for each NUMS generator, plus the blinding factor generator, how many multiples to precompute
 */
SECP256K1_API secp256k1_bulletproof_generators *secp256k1_bulletproof_generators_create(
    const secp256k1_context* ctx,
    const secp256k1_generator *blinding_gen,
    size_t n,
    size_t precomp_n
) SECP256K1_ARG_NONNULL(1);

/** Destroys a list of NUMS generators
 *  Args:   ctx: pointer to a context object (cannot be NULL)
 *          gen: pointer to the generator set to be destroyed
 */
SECP256K1_API void secp256k1_bulletproof_generators_destroy(
    const secp256k1_context* ctx,
    secp256k1_bulletproof_generators *gen
) SECP256K1_ARG_NONNULL(1);

/** Verifies a single bulletproof (aggregate) rangeproof
 *  Returns: 1: rangeproof was valid
 *           0: rangeproof was invalid, or out of memory
 *  Args:       ctx: pointer to a context object initialized for verification (cannot be NULL)
 *          scratch: scratch space with enough memory for verification (cannot be NULL)
 *             gens: generator set with at least 2*nbits*n_commits many generators (cannot be NULL)
 *  In:       proof: byte-serialized rangeproof (cannot be NULL)
 *             plen: length of the proof
 *        min_value: array of minimum values to prove ranges above, or NULL for all-zeroes
 *           commit: array of pedersen commitment that this rangeproof is over (cannot be NULL)
 *        n_commits: number of commitments in the above array (cannot be 0)
 *            nbits: number of bits proven for each range
 *        value_gen: generator multiplied by value in pedersen commitments (cannot be NULL)
 *     extra_commit: additonal data committed to by the rangeproof
 * extra_commit_len: length of additional data
 */
SECP256K1_API int secp256k1_bulletproof_rangeproof_verify(
    const secp256k1_context* ctx,
    secp256k1_scratch_space* scratch,
    const secp256k1_bulletproof_generators *gens,
    const unsigned char* proof,
    size_t plen,
    const uint64_t* min_value,
    const secp256k1_pedersen_commitment* commit,
    size_t n_commits,
    size_t nbits,
    const secp256k1_generator* value_gen,
    const unsigned char* extra_commit,
    size_t extra_commit_len
) SECP256K1_ARG_NONNULL(1) SECP256K1_ARG_NONNULL(2) SECP256K1_ARG_NONNULL(3) SECP256K1_ARG_NONNULL(4) SECP256K1_ARG_NONNULL(7) SECP256K1_ARG_NONNULL(10);

/** Batch-verifies multiple bulletproof (aggregate) rangeproofs of the same size using same generator
 *  Returns: 1: all rangeproofs were valid
 *           0: some rangeproof was invalid, or out of memory
 *  Args:       ctx: pointer to a context object initialized for verification (cannot be NULL)
 *          scratch: scratch space with enough memory for verification (cannot be NULL)
 *             gens: generator set with at least 2*nbits*n_commits many generators (cannot be NULL)
 *  In:       proof: array of byte-serialized rangeproofs (cannot be NULL)
 *         n_proofs: number of proofs in the above array, and number of arrays in the `commit` array
 *             plen: length of every individual proof
 *        min_value: array of arrays of minimum values to prove ranges above, or NULL for all-zeroes
 *           commit: array of arrays of pedersen commitment that the rangeproofs is over (cannot be NULL)
 *        n_commits: number of commitments in each element of the above array (cannot be 0)
 *            nbits: number of bits in each proof
 *        value_gen: generator multiplied by value in pedersen commitments (cannot be NULL)
 *     extra_commit: array of additonal data committed to by the rangeproof
 * extra_commit_len: array of lengths of additional data
 */
SECP256K1_API int secp256k1_bulletproof_rangeproof_verify_multi(
    const secp256k1_context* ctx,
    secp256k1_scratch_space* scratch,
    const secp256k1_bulletproof_generators *gens,
    const unsigned char* const* proof,
    size_t n_proofs,
    size_t plen,
    const uint64_t* const* min_value,
    const secp256k1_pedersen_commitment* const* commit,
    size_t n_commits,
    size_t nbits,
    const secp256k1_generator* value_gen,
    const unsigned char* const* extra_commit,
    size_t *extra_commit_len
) SECP256K1_ARG_NONNULL(1) SECP256K1_ARG_NONNULL(2) SECP256K1_ARG_NONNULL(3) SECP256K1_ARG_NONNULL(4) SECP256K1_ARG_NONNULL(8);


/** Produces an aggregate Bulletproof rangeproof for a set of Pedersen commitments
 *  Returns: 1: rangeproof was successfully created
 *           0: rangeproof could not be created, or out of memory
 *  Args:       ctx: pointer to a context object initialized for signing and verification (cannot be NULL)
 *          scratch: scratch space with enough memory for verification (cannot be NULL)
 *             gens: generator set with at least 2*nbits*n_commits many generators (cannot be NULL)
 *  Out:      proof: byte-serialized rangeproof (cannot be NULL)
 *  In/out:    plen: pointer to size of `proof`, to be replaced with actual length of proof (cannot be NULL)
 *  In:       value: array of values committed by the Pedersen commitments (cannot be NULL)
 *        min_value: array of minimum values to prove ranges above, or NULL for all-zeroes
 *            blind: array of blinding factors of the Pedersen commitments (cannot be NULL)
 *        n_commits: number of entries in the `value` and `blind` arrays
 *        value_gen: generator multiplied by value in pedersen commitments (cannot be NULL)
 *            nbits: number of bits proven for each range
 *            nonce: random 32-byte seed used to derive blinding factors (cannot be NULL)
 *     extra_commit: additonal data committed to by the rangeproof
 * extra_commit_len: length of additional data
 */
SECP256K1_API int secp256k1_bulletproof_rangeproof_prove(
    const secp256k1_context* ctx,
    secp256k1_scratch_space* scratch,
    const secp256k1_bulletproof_generators *gens,
    unsigned char* proof,
    size_t* plen,
    const uint64_t *value,
    const uint64_t *min_value,
    const unsigned char* const* blind,
    size_t n_commits,
    const secp256k1_generator* value_gen,
    size_t nbits,
    const unsigned char* nonce,
    const unsigned char* extra_commit,
    size_t extra_commit_len
) SECP256K1_ARG_NONNULL(1) SECP256K1_ARG_NONNULL(2) SECP256K1_ARG_NONNULL(3) SECP256K1_ARG_NONNULL(4) SECP256K1_ARG_NONNULL(5) SECP256K1_ARG_NONNULL(6) SECP256K1_ARG_NONNULL(8) SECP256K1_ARG_NONNULL(10) SECP256K1_ARG_NONNULL(12);

# ifdef __cplusplus
}
# endif

#endif
