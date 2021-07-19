{{-- resources/views/admin/dashboard.blade.php --}}

@extends('adminlte::page')

@section('title', 'Registros')

@section('content')
    <div class="card text-center">
        <div class="card-header">
            <h3><strong>Histórico de Registros de {{ $user->u }}</strong></h3>
        </div>
        <div class="card-body">
            <table class="table table-hover">
                <thead>
                  <tr>
                    <th scope="col">Nome</th>
                    <th scope="col">Dia</th>
                    <th scope="col">Entrada</th>
                    <th scope="col">Saída</th>
                  </tr>
                </thead>
                <tbody>
                  @foreach ($registers as $register)
                    <tr>   
                        <td>{{ $register['user']  }}</td>
                        <td>{{ $register['date']  }}</td>
                        <td>{{ $register['first']  }}</td>
                        <td>{{ $register['last']  }}</td>       
                    </tr> 
                  @endforeach
                </tbody>
              </table>
        </div>
        <div class="card-footer text-muted">
            <strong> Você é muito importante para nós!</strong>
        </div>
    </div>
@stop