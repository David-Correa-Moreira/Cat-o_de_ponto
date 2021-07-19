<?php

namespace App\Http\Controllers\Admin;

use App\Http\Controllers\Controller;
use App\Models\U;
use App\Models\User;
use Carbon\Carbon;
use Carbon\CarbonImmutable;
use Carbon\CarbonPeriod;
use Illuminate\Http\Request;
use App\Http\Requests\StoreUpdateURequest;

class UserController extends Controller
{
    private $repositor;

    public function __construct(U $u)
    {
        $this->repositor = $u;
    }
    
    public function index()
    {
        //dd(auth()->user());
        if ( auth()->user()->id == 1  ) {
            $users = $this->repositor->paginate();
        // $users = auth()->user();
        } else {
            $users = $this->repositor->whereU(auth()->user()->name)->paginate();
        }

        return view('control.projects.registers.index', ['users' => $users]);
    }

    public function create()
    {
        return view('control.projects.registers.create');
    }

    public function store(StoreUpdateURequest $request)
    {
        /*
        dd($request->all());
        $nome['u'] = "{$request->nome} {$request->sobrenome}";
        $nome['s'] = $request->status;
        //dd($nome);
        $this->repositor->create($nome);
        */

        /**
         * recuperou os dados do pelo request e criou um objeto da tabela user(pertencente ao laravel).
         * Atraves do relacionamento da tabela user com a tabela u, criou usuario arduino
         * .
         */
        $user = User::create([
            'name'          => "{$request->nome} {$request->sobrenome}",
            'email'         => $request->email,
            'password'      => bcrypt($request->password)
        ]);

        $user->u()->create(['s' => $request->status]);

        return redirect()->route('registers.index');
    }

    public function show($id)
    {
        $user = $this->repositor->find($id);

        $dataBase = CarbonImmutable::today(); // Captura a data atual

        $periodo = CarbonPeriod::between($dataBase->startOfMonth(), $dataBase->endOfMonth()); // Monta um array com todos os dias do mês

        $registers = [];

        foreach ($periodo as $data) { // Percorre cada dia do mês
            $primeiroRegistro =  $user->r()->whereDate('created_at', $data->format('Y-m-d'))->orderBy('id', 'asc')->first();
            $UltimoRegistro =  $user->r()->whereDate('created_at', $data->format('Y-m-d'))->orderBy('id', 'desc')->first();

            if ($primeiroRegistro) {
                $registers[] = [
                    'user' => $user->u,
                    'date' => $data->format('d/m/Y'),
                    'first' => $primeiroRegistro->created_at->format('H:i:s'),
                    'last' => $UltimoRegistro->created_at->format('H:i:s')
                ];
            }
        }

        return view('control.projects.registers.show', compact('user', 'registers'));
    }

    public function delete($id)
    {
        $user = $this->repositor->where('id', $id)->first();
        
        if (!$user)
           return redirect()->back();
        
        $user->delete();

        return redirect()->route('registers.index');
    }

    public function edit($id)
    {
        $user = $this->repositor->where('id', $id)->first();

        if (!$user)
           return redirect()->back();

        $fullName = explode(' ', $user->u);
        $user['nome'] = $fullName[0];
        $user['sobrenome'] = $fullName[1];
        unset($user['u']);
        //dd($user);
        
        return view('control.projects.registers.edit', compact('user'));
    }

    public function update(StoreUpdateURequest $request, $id )
    {

        $user = $this->repositor->where('id', $id)->first();

        if (!$user)
            return redirect()->back();

          
        $teste['id'] = $request->id;
        $teste['u'] = "{$request->nome} {$request->sobrenome}";
        $teste['s'] = $request->status;
        
        //$user = $request->all();  
        //$user->u= "{$request->nome} {$request->sobrenome}";
        //$user->s = $request->s;
        //dd($user);

        //$this->repositor->update($user);
        $user->update($teste);

        return redirect()->route('registers.index');
        
    }
}
