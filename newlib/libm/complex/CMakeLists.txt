# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
picolibc_sources(
  cabs.c
  cacos.c
  cacosh.c
  carg.c
  casin.c
  casinh.c
  catan.c
  catanh.c
  ccos.c
  ccosh.c
  cephes_subr.c
  cexp.c
  cimag.c
  clog.c
  clog10.c
  conj.c
  cpow.c
  cproj.c
  creal.c
  csin.c
  csinh.c
  csqrt.c
  ctan.c
  ctanh.c
  cabsf.c
  casinf.c
  ccosf.c
  cimagf.c
  cprojf.c
  csqrtf.c
  cacosf.c
  casinhf.c
  ccoshf.c
  clogf.c
  clog10f.c
  crealf.c
  ctanf.c
  cacoshf.c
  catanf.c
  cephes_subrf.c
  conjf.c
  csinf.c
  ctanhf.c
  cargf.c
  catanhf.c
  cexpf.c
  cpowf.c
  csinhf.c
  )

if(${_HAVE_LONG_DOUBLE})
  picolibc_sources(
    cabsl.c
    creall.c
    cimagl.c
    conjl.c
    cprojl.c
    csqrtl.c
    cacoshl.c
    cacosl.c
    cargl.c
    casinhl.c
    casinl.c
    catanhl.c
    catanl.c
    cephes_subrl.c
    ccoshl.c
    ccosl.c
    cexpl.c
    clog10l.c
    clogl.c
    cpowl.c
    csinhl.c
    csinl.c
    ctanhl.c
    ctanl.c
    )
endif()
